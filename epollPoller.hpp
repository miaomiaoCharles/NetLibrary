#pragma once
#include "poller.hpp"
#include <sys/epoll.h>
#include "Logger.hpp"
class EpollPoller: public Poller{
public:
    EpollPoller(EventLoop* loop); //epoll_create
    ~EpollPoller() override;
    //重写基类的抽象方法
    TimeStamp poll(int timeoutMs, ChannelList* activeChannels) override; //epoll_wait()
    void updateChannel(Channel* channel) override;  //epoll_ctl add/modify
    void removeChannel(Channel* channel) override;   //epoll_ctl del
private:
    static const int kInitEventListSize = 16; //muduo中，发生感兴趣事件的文件描述符，装在_events里，这个vector初始长度是16
    //填写活跃的连接，（告诉该管道你感兴趣的事件发生啦）也是将_events转成channels
    void fillActiveChannnels(int numEvents, ChannelList* activeChannnels) const;
    void update(int operation, Channel* channel); 

    using EventList = vector<epoll_event>;
 
    int _epollfd;
    EventList _events; //它用来接收epoll_wait调用返回来的epoll_event，这些都是感兴趣事件发生的event
};