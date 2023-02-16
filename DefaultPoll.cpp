#include "poller.hpp"
#include "epollPoller.hpp"
//code
Poller* Poller::newDefaultPoller(EventLoop* loop){
    if(::getenv("MUDUO_USE_POLL")){
        return nullptr;
    }else{
        return new EpollPoller(loop);
    }
}