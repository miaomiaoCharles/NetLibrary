#pragma once
using namespace std;
#include <functional>
#include <memory>
#include "TimeStamp.hpp"
#include "noncopyable.hpp"

//channnel就是通道，封装了sockedfd和其感兴趣的event，还绑定了poller返回的具体事件
class EventLoop;
class Channel:Noncopyable{
public:
    using EventCallback = function<void()>;
    using ReadEventCallback = function<void(TimeStamp)>;
    Channel(EventLoop* loop, int fd);
    ~Channel();
    void handleEvent(TimeStamp receiveTime); //fd得到poller通知后，通过调用这个来处理事件

    //设置回调函数对象
    void setReadCallback(ReadEventCallback cb){ _readCallback = std::move(cb); }
    void setWriteCallback(EventCallback cb){ _writeCallback = std::move(cb); }
    void setCloseCallback(EventCallback cb){ _closeCallback = std::move(cb);}
    void setErrorCallback(EventCallback cb){ _errorCallback = std::move(cb); }
    //防止当channel被手动remove，channel还在执行回调
    void tie(const shared_ptr<void>&);
    int fd()const {return _fd;}
    int events() const{return _events;} //设置该chennel感兴趣的事件
    void set_revent(int revents){_revents = revents;} //当poller监听到相应的事件时候，会把该【事件告诉通道】，让通道去调用相应的回调函数
    //设置fd相应的事件状态，对什么感兴趣
    void enableReading(){_events |= kReadEvent; update();} //updata本质就是epoll_ctl去添加感兴趣的事件
    void disableReading(){_events &= ~kReadEvent; update();}
    void enableWriting(){_events |= kWriteEvent; update();}
    void disableWriting(){_events &= ~kWriteEvent; update();}
    void disableAll(){_events = kNoneEvent; update();}
    //返回当前fd状态,对哪个感兴趣
    bool isNoneEvent()const{return _events == kNoneEvent;}
    bool isWriting()const{return _events & kWriteEvent;}
    bool isReading() const {return _events&kReadEvent;}

    int index(){return _index;}
    void setIndex(int index){_index = index;}
    //返回该channel所属的eventloop
    EventLoop* ownerLoop() {return _loop;}
    void remove();
private:
    void update();
    void handleEventWithGuard(TimeStamp receiveTime);
    //表示当前fd的状态，对哪个感兴趣
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    
    EventLoop* _loop; //事件循环
    const int _fd; //poller监听的套接字
    int _events; // fd感兴趣的事件
    int _revents; //poller返回 具体发生的事件 
    int _index;

    weak_ptr<void> _tie;
    bool _tied;
    //回调函数，因为通道能够获知fd具体的事件revents，所以它负责具体事件的回调操作。
    ReadEventCallback _readCallback;
    EventCallback _writeCallback;
    EventCallback _closeCallback;
    EventCallback _errorCallback;
};