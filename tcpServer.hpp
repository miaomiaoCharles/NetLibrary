#pragma once
#include "noncopyable.hpp"
#include "eventLoop.hpp"
#include "InetAddress.hpp"
#include "tcpConnection.hpp"
#include "buffer.hpp"
#include "eventLoopThreadPool.hpp"
#include "Acceptor.hpp"
#include "callBack.hpp"
#include <atomic>
#include <unordered_map>
using namespace std;
//面向用户的类，主要驱动accptor
class TcpServer:Noncopyable{
public:
    using ThreadInitCallback = function<void(EventLoop*)>; //线程创建时候执行的一个函数，可以执行初始化loop等操作
    using ConnectionMap = unordered_map<string, TcpConnectionPtr>;
    enum Option  
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& name, Option option = kNoReusePort);
    ~TcpServer();
    void setThreadInitCallBack(ThreadInitCallback cb){_threadInitCallback = cb;}
    void setConnectionCallBack(ConnectionCallBack cb){_connectionCallBack = cb;}
    void setMessageCallBack(MessageCallBack cb){_messageCallBack = cb;}
    void setWriteCompleteCallBack(WriteCompleteCallBack cb){_writeCompleteCallBack = cb;}
    void setThreadNum(int num);
    void start(); //accptor的listen开启
private:
    void newConnection(int, const InetAddress&);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn );
    EventLoop* _loop;
    string _ipPort;
    string _name;
    unique_ptr<Acceptor> _acceptor; //acceptr运行在mainLoop的线程上，主要任务就是监听新连接事件
    shared_ptr<EventLoopThreadPool> _threadPool;
    ConnectionCallBack _connectionCallBack;
    MessageCallBack _messageCallBack;
    WriteCompleteCallBack _writeCompleteCallBack; //消息发送完成后的回调
    ThreadInitCallback _threadInitCallback;
    atomic_int  _started; 
    int _nextConnId; //记录客户端链接数量
    ConnectionMap _connections; //保存所有连接
};