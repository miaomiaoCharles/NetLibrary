#include "eventLoopThreadPool.hpp"
#include "eventLoopThread.hpp"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg)
:_baseLoop(baseLoop)
,_name(nameArg)
,_started(false)
,_numThreads(0)
,_next(0)
{

}
EventLoopThreadPool::~EventLoopThreadPool(){};
void EventLoopThreadPool::start(const threadInitCallBack& cb){
    _started = true;
    for(int i = 0; i < _numThreads; i++){
        char buf[_name.size()+32];
        snprintf(buf, sizeof buf , "%s%d", _name.c_str(), i);
        EventLoopThread* t =   new EventLoopThread(cb, buf);
        _allThreads.push_back(unique_ptr<EventLoopThread>(t));
        _allLoops.push_back(t->startLoop()); //底层创建新线程，每一个子线程开始执行
    }
    if(_numThreads == 0 && cb){ //说明用户就没用setThreadNum这个函数，那么自始至终就只好让baseLoop去单线程执行所有任务了
        cb(_baseLoop);
    }
}
//多线程中轮询选择subLoop去分配channel给他们
EventLoop* EventLoopThreadPool::getNextLoop(){
    EventLoop* loop = _baseLoop;
    if(!_allLoops.empty()){
        loop = _allLoops[_next];
        ++ _next;
        if(_next >= _allLoops.size()){
            _next = 0;
        }
    }
    return loop; 
}
vector<EventLoop*> EventLoopThreadPool::getAllLoops(){
    if(_allLoops.empty()){
        return vector<EventLoop*>(1, _baseLoop); //如果就一个baseLoop，就把baseLoop装vector里返回去,否则返回所有的subLoop
    }else{
        return _allLoops;
    }
}