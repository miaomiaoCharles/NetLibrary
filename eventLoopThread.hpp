#pragma once

#include "noncopyable.hpp"
#include "thread.hpp"
#include "eventLoop.hpp"
#include <functional>
#include <mutex>
#include <condition_variable>
//关于这个类，我怀疑就是mainLoop在创建subloop，mainLoop的线程通过多次运行这个类里的startLoop去创建工作线程。
class EventLoopThread:Noncopyable
{
public:
    using threadInitCallBack = function<void(EventLoop*)>; //运行子线程创建eventLoop后，可以对新建立的loop进行一些操作

    EventLoopThread(const threadInitCallBack &cb, const string& name = string());
    ~EventLoopThread();
    EventLoop* startLoop(); //建立新线程，并在新线程里创建loop并开始循环，返回新建立的loop
private:
    void threadFunc(); //启动一个新线程不是要传入一个线程函数嘛，就传它
    EventLoop* _loop;
    bool _exiting;
    Thread _thread;
    mutex _mutex;
    condition_variable _cond;
    threadInitCallBack _callback;
};