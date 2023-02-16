#include "eventLoop.hpp"
#include "Logger.hpp"
#include "channel.hpp"
#include <sys/eventfd.h>
//防止一个线程创建多个eventlopp 如果这个指针不为空就会停止创建
__thread EventLoop* t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000; //超时时间

//创建wakeupfd,用来notify唤醒subReactor处理新来的channel
int createEventfd(){
    int evtfd = eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC);
    if(eventfd < 0){
        LOG_FATAL("eventfd error:%d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : _looping(false)
    , _quit(false)
    , _callingPendingFunctors(false)
    , _threadId(currentThread::tid())
    , _poller(Poller::newDefaultPoller(this))
    , _wakeupFd(createEventfd())
    , _wakeupChannel(new Channel(this, _wakeupFd))
{
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, _threadId);
    if(t_loopInThisThread){
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, _threadId); //一个进程只能有一个eventloop
    }else{
        t_loopInThisThread = this;
    }
    //设置wakeupfd的事件类型以及发生事件后的回调操作
    _wakeupChannel->setReadCallback(bind(&EventLoop::handleRead, this));
    _wakeupChannel->enableReading();
}

void EventLoop::handleRead(){ //当wakeupfd可以读时候，回调这个函数   
    uint64_t one = 1;
    ssize_t n = read(_wakeupFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8", n);
    }
}

EventLoop::~EventLoop(){
    _wakeupChannel->disableAll();
    _wakeupChannel->remove();
    ::close(_wakeupFd);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop(){
    _looping = true;
    _quit = false;
    LOG_INFO("Eventloop %p start looping \n", this);

    while(!_quit){
        _activeChannels.clear();
        //workLoop会在这里监听两类fd，一类是wakefd可以读（新的channel来啦），一类是它监听的管道中有感兴趣的事情发生了
        _pollReturnTime = _poller->poll(kPollTimeMs, &_activeChannels);
        for(Channel* channel: _activeChannels){
            channel->handleEvent(_pollReturnTime); //要是wakefd,就是执行回调读操作，读取八个字节就行（关键是唤醒subLopp）其他的就是用户设置的回调
        }
        //执行当前eventlopp事件循环需要处理的回调操作。就比如说需要subLoop被唤醒后，它可能是因为wakeupfd唤醒的，需要去执行接收新channel的操作
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping. \n", this);
    _looping = false;
}

void EventLoop::quit(){
    _quit = true;
    //如果是自己线程调用这个函数，它肯定不在poll阻塞中，直接quit=true就行了,会在while条件处退出循环。
    //但是如果是一个线程调用另一个线程eventloop的quit，这个线程很可能还在阻塞呢，直接调用quit并不会使它退出while循环，因此要先唤醒
    if(!isInLoopThread()){
        wakeup();
    }
}

//在当前loop中执行cb
void EventLoop::runInLoop(Functor cb){
    if(isInLoopThread()){ //在当前的loop线程中，执行cb
        cb();
    }else{
        queueInLoop(cb); //在非当前loop线程中执行cb，就需要唤醒loop所在线程，执行cb。比如说在subLoop1的线程里调用subLoop2的回调，subLoop2就需要被唤醒
    }
}
//把cb放入队列中，唤醒loop所在的线程，执行cb
void EventLoop::queueInLoop(Functor cb){
    {
        unique_lock<mutex> lock(_mutex); //因为可能很多个loop都要同时调用这个loop的函数
        _pendingFunctors.emplace_back(cb);//stl没有线程安全设计
    }
    //唤醒相应的需要执行回调操作的loop的线程，让它去执行
    // _callingPendingFunctors：如果那个执行回调的线程正在运行别的线程给他的回调，那么它执行完了转到poll()又会阻塞住，因此要wakeup写入，它刚刚阻塞住就会释放
    if(!isInLoopThread() || _callingPendingFunctors){
        wakeup(); //唤醒loop所在线程
    }
}

void EventLoop::updateChannel(Channel* channel){
    _poller->updateChannel(channel);
}
void EventLoop::removeChannel(Channel* channel){
    _poller->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel* channel){
    return _poller->hasChannel(channel);
}

//往wakeupfd里写数据，从而使线程不再阻塞到poll()
void EventLoop::wakeup(){
    uint64_t one = 1;
    auto n = write(_wakeupFd, &one, sizeof one);
}
//执行回调
void EventLoop:: doPendingFunctors(){
    vector<Functor> functors;
    _callingPendingFunctors = true;
    {
        unique_lock<mutex> lock(_mutex);
        functors.swap(_pendingFunctors);  
        // 为什么要新建立一个vector，把之前的vector的函数全放新vector里？ 
        //因为，如果如果还用原来的vector，整个过程就要加锁（取函数执行），如果这个时间比较长，别的线程就无法往旧vector里放函数，会饥饿
    }
    for(auto functor: functors){
        functor();   // 执行当前loop需要执行的回调操作
    }
    _callingPendingFunctors = false;
} 