#pragma once
#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include "poller.hpp"
#include "TimeStamp.hpp"
#include "currentThread.hpp"
#include "channel.hpp"
using namespace std;

//事件循环类，主要包含channel， poller（epool）
//对应reactor
class EventLoop{
public:
    using Functor = function<void()>;
    EventLoop();
    ~EventLoop();
    //开启事件循环
    void loop();
    //退出事件循环
    void quit();

    TimeStamp pollReturnTime() const{return _pollReturnTime;}
    //在当前loop中执行cb
    void runInLoop(Functor cb);
    //把cb放入队列中，唤醒loop所在的线程，执行cb
    void queueInLoop(Functor cb);

    // 用来唤醒loop所在的线程的，本质是向wakeupfd写一个数据
    void wakeup();

    // EventLoop的方法 =》 Poller的方法
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    // 判断EventLoop对象是否在自己的线程里面
    bool isInLoopThread() const { return _threadId ==  currentThread::tid(); }
private:
    void handleRead(); //唤醒工作线程
    void doPendingFunctors(); //执行回调

    using ChannelList = vector<Channel*>;

    atomic_bool _looping;
    atomic_bool _quit;

    const pid_t _threadId ;//记录当前loop所在线程的id，创立这个loop时候的线程id
    TimeStamp _pollReturnTime;//poller返回发生事件的channels的时间点
    unique_ptr <Poller> _poller;

    int _wakeupFd; // 当mainloop获取一个新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
    unique_ptr <Channel> _wakeupChannel;

    ChannelList _activeChannels; //所有的发生事件的channel；
    
    atomic_bool _callingPendingFunctors; // 标识当前loop是否有需要执行的回调操作   
    vector<Functor> _pendingFunctors; // 存储loop需要执行的所有的回调操作
    mutex _mutex; // 互斥锁，用来保护上面vector容器的线程安全操作
};