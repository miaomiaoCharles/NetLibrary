#include "eventLoopThread.hpp"

EventLoopThread::EventLoopThread(const threadInitCallBack &cb, const string& name)
: _loop(nullptr)
, _exiting(false)
, _thread(bind(&EventLoopThread::threadFunc, this), name)
, _mutex()
, _cond()
, _callback(cb)
{

}
EventLoopThread::~EventLoopThread(){
    _exiting = false;
    if(_loop != nullptr){
        _loop->quit();
        _thread.join();  //主线程等待它创立的子线程结束运行
    }
}

//建立新线程，并在新线程里创建loop并开始循环，返回新建立的loop
EventLoop* EventLoopThread::startLoop(){
    _thread.start(); //子线程去运行了
    //主线程继续往下走
    EventLoop* loop = nullptr;
    {
        unique_lock<mutex> lock(_mutex);
        while(_loop == nullptr){
            _cond.wait(lock);  //使用条件变量来保证子线程运行到了给_loop赋值那一步
        }
        loop = _loop;
    }
    return loop;
}
//启动一个新线程不是要传入一个线程函数嘛，就传它。这是子线程去建立eventLoop的过程
void EventLoopThread::threadFunc(){
    EventLoop loop; //建立一个新的线程
    if(_callback){
        _callback(&loop);
    }
    {
        unique_lock<mutex> lock(_mutex);
        _loop = &loop;
        _cond.notify_one();
    }
    loop.loop();
    unique_lock<std::mutex> lock(_mutex); //子线程的loop退出循环了
    _loop = nullptr;
}