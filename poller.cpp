#include "poller.hpp"
#include "channel.hpp"
Poller::Poller(EventLoop* loop): _ownerloop(loop){}

bool Poller::hasChannel(Channel* channel) const{
    auto it = _channels.find(channel->fd());
    return (it != _channels.end()) && (it->second == channel);
}

