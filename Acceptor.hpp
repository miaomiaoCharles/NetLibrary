#pragma once
#include "noncopyable.hpp"
#include "socket.hpp"
#include "channel.hpp"
#include "InetAddress.hpp"
#include <functional>
using namespace std;
//acceptor的作用，这是很关键的模块，连接了mainLoop和subLoop，它封装管道交给mainloop，一有新用户连接，管道的回调函数启动(给subLoop下发channel)
class Acceptor:Noncopyable{
public:
    using newConnectionCallBack = function<void(int socketFd, const InetAddress&)>;
    //传入的地址是自己的地址
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();
    void setNewConnectionCallBack(const newConnectionCallBack& cb){
        _newConnectionCallBack = move(cb);
    }
    bool listenning(){return _listenning;}
    void listen();
private:
    void handleRead();  //当有新用户连接时候回启动这个函数，它会调用TcpServer传进来的回调函数来分发channel等,他是直接传入channel的函数

    EventLoop* _loop; //baseLoop
    Socket _acceptSocket; // 一个socket用来监听新用户连接
    Channel _acceptChannel; //把socket包装一下，把这个channel给baseLoop，这样当新用户连接时候才能启动相应回调。
    newConnectionCallBack  _newConnectionCallBack; //TCPServer传入的回调，里面主要做分发channel给subLoop的操作
    bool _listenning;
};