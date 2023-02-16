#pragma once
#include "noncopyable.hpp"
#include "InetAddress.hpp"
#include "Logger.hpp"

#include <unistd.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
using namespace std;
 //封装了fd，同时也增加了绑定和accept的功能
class Socket:Noncopyable{
public:
    explicit Socket(int socketFd):_socketFd(socketFd){}
    ~Socket();
    int fd();
    void bindAddress(const InetAddress& address);
    void listen();
    int accept(InetAddress* address); //从全连接队列里取出套接字，并把对应的客户端地址放入address中
    void shutdownWrite();
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int _socketFd;
} ;