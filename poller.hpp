#pragma once
#include "channel.hpp"
#include "TimeStamp.hpp"
#include <vector>
#include <unordered_map>
#include "channel.hpp"
using namespace std;
class EventLoop;
class Poller{
public:
    using ChannelList = vector<Channel*>;
    Poller(EventLoop* loop);
    virtual ~Poller() = default;
    //以下虚函数为具体的io复用保留的统一接口
    virtual TimeStamp poll(int timeoutMs, ChannelList* activeChannels) = 0; //启动epoll_wait()
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;
    //判断这个channel是否在这个poller中
    bool hasChannel(Channel* channel) const;
    //获取具体的io复用，比如说epoll或者poll。注意我们不希望在基类里面包含派生类的头文件，所以专门开辟一个cpp文件去实现它
    static Poller* newDefaultPoller(EventLoop* loop);
protected:
    //key代表的是这个channel的socket，value就是这个channel的指针，保存了所有的channel
    using ChannelMap = unordered_map<int, Channel*>;
    ChannelMap _channels;
private:
    EventLoop* _ownerloop; //所属的eventloop
};