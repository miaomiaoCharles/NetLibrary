#pragma once
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;
class InetAddress{
public:
    explicit InetAddress(const sockaddr_in& addr):_addr(addr){}
    explicit InetAddress(uint16_t port, string ip = "127.0.0.1");
    InetAddress(){}
    string toIP() const;
    string toIpPort() const;
    uint16_t toPort() const;
    const sockaddr_in getSockaddr_in() const;
    void setAddr(const sockaddr_in& addr);
private:
    sockaddr_in _addr;
};