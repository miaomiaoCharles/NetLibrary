#include "InetAddress.hpp"
#include <strings.h>
#include<string.h>
InetAddress::InetAddress(uint16_t port, string ip){
    bzero(&_addr, sizeof _addr);
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);  //将port转成网络字节序
    _addr.sin_addr.s_addr = inet_addr(ip.c_str()); //将ip转成网络字节序存储起来
}
string InetAddress::toIP() const{
    char buf[64] = {0};
    inet_ntop(AF_INET, &_addr.sin_addr, buf, sizeof buf);
    return buf;
}
string InetAddress::toIpPort() const{
    char buf[64] = {0};
    inet_ntop(AF_INET, &_addr.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(_addr.sin_port);
    sprintf(buf+end, ":%u", port);
    return buf;
}
uint16_t InetAddress::toPort() const{
    return ntohs(_addr.sin_port);
}
const sockaddr_in InetAddress::getSockaddr_in() const{
    return _addr;
}
void InetAddress::setAddr(const sockaddr_in& addr){
    _addr = addr;
}