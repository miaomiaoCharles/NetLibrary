#include "thread.hpp"
atomic_int Thread:: _numCreated(0);
Thread::Thread(ThreadFunc func, const string &name)
    : _started(false)
    , _joined(false)
    , _tid(0)
    , _func(move(func)) //不使用拷贝构造，而使用move，这里用的是move的第二个功能（浅拷贝并销毁前对象的指针）
    , _name(name)
{
    setDefaultName();
}

Thread::~Thread(){
    if(_started && _joined){
        _thread->detach(); //这样的话，就算子线程没有执行完，主线程也可以结束而不会报错
    }
}

void Thread::start(){
    _started = true;
    sem_t sem;
    sem_init(&sem,false,0);

    //开启线程
    _thread = shared_ptr<thread>(new thread([&](){
        _tid = currentThread::tid();
        sem_post(&sem);
        _func();
    }));

    //必须等待上面新创建的线程获取到tid值,才结束start函数。
    sem_wait(&sem);
}
void Thread::join()
{
    _joined = true;
    _thread->join();
}
void Thread::setDefaultName()
{
    int num = ++_numCreated;
    if (_name.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        _name = buf;
    }
}