#pragma once

#include "noncopyable.hpp"
#include "currentThread.hpp"
#include <thread>
#include <atomic>
#include <memory>
#include <functional>
#include <semaphore.h>
using namespace std;
class Thread: Noncopyable{
public:
    using ThreadFunc = function<void()>; //通过bind可以改变线程函数的参数数量

    explicit Thread(ThreadFunc, const string& name = string());
    ~Thread();

    void start();
    void join();

    bool started() const {return _started;}
    pid_t tid() const{return _tid;}
    const string name() const{return _name;}

    static int numCreated(){return _numCreated;}

private:
    void setDefaultName();
    bool _started;
    bool _joined;
    shared_ptr<thread> _thread;
    pid_t _tid;
    ThreadFunc _func;
    string _name;
    static atomic_int _numCreated;
};