#pragma once
#include "noncopyable.hpp"
#include "InetAddress.hpp"
#include "callBack.hpp"
#include "buffer.hpp"
#include "TimeStamp.hpp"

#include <memory>
#include <string>
#include <atomic>

class Channel;
class EventLoop;
class Socket;
using namespace std;

class TcpConnection:Noncopyable, public enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop, 
            const string &name, 
            int sockfd,
            const InetAddress& localAddr,
            const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return _loop; }
    const string& name() const { return _name; }
    const InetAddress& localAddress() const { return _localAddr; }
    const InetAddress& peerAddress() const { return _peerAddr; }

    bool connected() const { return _state == kConnected; }

    // 发送数据
    void send(const string &buf);
    // 关闭连接
    void shutdown();
    void setConnectionCallback(const ConnectionCallBack& cb)
    { _connectionCallback = cb; }

    void setMessageCallback(const MessageCallBack& cb)
    { _messageCallback = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallBack& cb)
    { _writeCompleteCallback = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallBack& cb, size_t highWaterMark)
    { _highWaterMarkCallback = cb; _highWaterMark = highWaterMark; }

    void setCloseCallback(const CloseCallBack& cb)
    { _closeCallback = cb; }

    // 连接建立
    void connectEstablished();
    // 连接销毁
    void connectDestroyed();

private:
    enum StateE {kDisconnected, kConnecting, kConnected, kDisconnecting};
    void setState(StateE state) { _state = state; }

    void handleRead(TimeStamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* data, size_t len);

    void shutdownInLoop();

    EventLoop * _loop; // 这里绝对不是baseLoop， 因为TcpConnection都是在subLoop里面管理的
    const string _name;
    atomic_int _state;
    bool _reading;

    // 这里和Acceptor类似   Acceptor=》mainLoop    TcpConenction=》subLoop
    unique_ptr<Socket> _socket;
    unique_ptr<Channel> _channel;

    const InetAddress _localAddr;
    const InetAddress _peerAddr;

    ConnectionCallBack _connectionCallback; // 有新连接时的回调
    MessageCallBack _messageCallback; // 有读写消息时的回调
    WriteCompleteCallBack _writeCompleteCallback; // 消息发送完成以后的回调
    HighWaterMarkCallBack _highWaterMarkCallback;
    CloseCallBack _closeCallback;
    size_t _highWaterMark;

    Buffer _inputBuffer;  // 接收数据的缓冲区
    Buffer _outputBuffer; // 发送数据的缓冲区

};