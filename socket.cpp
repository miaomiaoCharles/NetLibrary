#include "socket.hpp"

Socket:: ~Socket(){
    close(_socketFd);
}
int Socket::fd(){
    return _socketFd;
}
void Socket::bindAddress(const InetAddress& address){
    sockaddr_in addr = address.getSockaddr_in();
    if(bind(_socketFd, (sockaddr*)&addr, sizeof(sockaddr_in)) != 0){
        LOG_FATAL("bind sockfd:%d fail \n", _socketFd);
    }
}
void Socket::listen(){
    if(::listen(_socketFd,1024) != 0){
        LOG_FATAL("listen sockfd:%d fail \n", _socketFd);
    } 
}

int Socket::accept(InetAddress* address){
    sockaddr_in addr;
    socklen_t len = sizeof addr;
    bzero(&addr, len);
    int connfd = ::accept4(_socketFd, (sockaddr*)address, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    address->setAddr(addr);
    return connfd;
} 

void Socket::shutdownWrite()
{
    if (::shutdown(_socketFd, SHUT_WR) < 0)
    {
        LOG_ERROR("shutdownWrite error");
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(_socketFd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval); //允许套接字绑定到已在使用的地址
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(_socketFd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);//让端口释放后就可以立即被再次使用
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(_socketFd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}

