#include "epollPoller.hpp"
#include <unistd.h>
#include <errno.h>
#include <string.h>
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop* loop): Poller(loop){
    _epollfd = epoll_create1(EPOLL_CLOEXEC); //进程被替换时会关闭文件描述符
    _events.resize(kInitEventListSize);
    if(_epollfd < 0){
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}
EpollPoller::~EpollPoller(){
    ::close(_epollfd);
}
void EpollPoller::updateChannel(Channel* channel){ //修改逻辑看笔记本
    const int index = channel->index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d \n", __FUNCTION__, channel->fd(), channel->events(), index);
    if(index == kNew || index == kDeleted){
        if(index == kNew){
            int fd = channel->fd();
            _channels[fd] = channel;
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }else{  //channel已经在epolll上注册过了
        int fd = channel->fd();
        if(channel->isNoneEvent()){ //对任何事情不感兴趣
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        }else{
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
//从epoll中删除channel，这里的删除是要从map里也删掉，就跟没注册过一样
void EpollPoller::removeChannel(Channel* channel){
    int fd = channel->fd();
    _channels.erase(fd);
    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);
    if(channel->index() == kAdded){
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}

void EpollPoller::update(int operation, Channel* channel){ //这是个私有方法，用来封装系统调用的epoll_ctl方法
    epoll_event event;
    bzero(&event, sizeof event);

    int fd = channel->fd();

    event.events = channel->events(); //event 代表了一个epoll监听的事件，里面绑定了fd以及channnel， 还有感兴趣的事件
    event.data.fd = channel->fd();
    event.data.ptr = channel;
    if (::epoll_ctl(_epollfd, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}
//由eventloop调用，开始启动epollwait
TimeStamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels){
    LOG_INFO("func=%s => fd total count:%lu \n", __FUNCTION__, _channels.size());
    int numEvents = epoll_wait(_epollfd, &*_events.begin(), static_cast<int>(_events.size()), timeoutMs);
    int saveErrno = errno;
    TimeStamp now(TimeStamp::now());

    if(numEvents > 0){
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannnels(numEvents, activeChannels);
        if(numEvents == _events.size()){
            _events.resize(_events.size() * 2);//众所周知，_events初始只有16，然而如果发生感兴趣事件的event大于16，就会下一次再往_events里放（LT模式）
        }
    }else if(numEvents == 0){
        LOG_DEBUG("%s timeout! \n", __FUNCTION__);
    }else{
        if (saveErrno != EINTR)
            {
                errno = saveErrno;
                LOG_ERROR("EPollPoller::poll() err!");
            }
        }
    return now;
}

void EpollPoller::fillActiveChannnels(int numEvents, ChannelList* activeChannnels)const{
    for(int i = 0; i < numEvents; i++){
        Channel* channel = static_cast<Channel*>(_events[i].data.ptr);
        channel->set_revent(_events[i].events);
        activeChannnels->push_back(channel);
    }
}