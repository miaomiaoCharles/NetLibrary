#pragma once
#include "noncopyable.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>
using namespace std;

class EventLoop;
class EventLoopThread;
class EventLoopThreadPool:Noncopyable
{
public:
    using threadInitCallBack = function<void(EventLoop*)>;  //创立eventloopthread需要的回调函数
    EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg);
    ~EventLoopThreadPool();
    void setNumThread(int n){_numThreads = n;}
    void start(const threadInitCallBack& cb = threadInitCallBack());
    EventLoop* getNextLoop();
    vector<EventLoop*> getAllLoops();
    bool isStarted(){return _started;}
    string name(){return _name;}
private:
    EventLoop* _baseLoop; //使用moduo时候用户创立的loop；
    string _name;
    bool _started;
    int _numThreads;
    int _next; //分配channel是由baseLoop循环给subLoop分配，next是下一个待分配的subLoop序号
    vector<unique_ptr<EventLoopThread> > _allThreads;
    vector<EventLoop*> _allLoops; //线程开始运行时候会返回EventLoop的
};