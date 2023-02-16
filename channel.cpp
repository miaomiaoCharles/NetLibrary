#include "channel.hpp"
#include "sys/epoll.h"
#include "Logger.hpp"
#include "eventLoop.hpp"
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN| EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd):_loop(loop), _fd(fd), _events(0), _revents(0), _index(-1), _tied(false){}

Channel::~Channel(){}
//什么时候绑定的呢？？  connection连接建立时候绑定
void Channel::tie(const shared_ptr<void>& obj){
    _tie = obj;
    _tied = true;
}
//通道自己是没有资格去epoll_ctl的，它要让loop去,code
void Channel::update(){
    _loop->updateChannel(this);
}
//同理，不能自己删除自己，要让loop去删除自己这个管道 code
void Channel::remove(){
    _loop->removeChannel(this);
}

void Channel::handleEvent(TimeStamp reciveTime){
    if(_tied){
        shared_ptr<void> guard = _tie.lock(); //提升不成功，说明这个管道已经remove掉了
        if(guard){
            handleEventWithGuard(reciveTime);
        }
    }else{
        handleEventWithGuard(reciveTime);
    }
}

void Channel::handleEventWithGuard(TimeStamp reciveTime){
    LOG_INFO("channel handleEvent revents:%d\n", _revents);

    if ((_revents & EPOLLHUP) && !(_revents & EPOLLIN))
    {
        if (_closeCallback)
        {
            _closeCallback();
        }
    }

    if (_revents& EPOLLERR)
    {
        if (_errorCallback)
        {
            _errorCallback();
        }
    }

    if (_revents & (EPOLLIN | EPOLLPRI))
    {
        if (_readCallback)
        {
            _readCallback(reciveTime);
        }
    }

    if (_revents & EPOLLOUT)
    {
        if (_writeCallback)
        {
            _writeCallback();
        }
    }
}