#include "tcpConnection.hpp"
#include "Logger.hpp"
#include "socket.hpp"
#include "channel.hpp"
#include "eventLoop.hpp"
#include <functional>
#include <errno.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <string>
static EventLoop* checkLoopNotNull(EventLoop* loop){
    if(loop == nullptr){
        LOG_FATAL("%s:%s:%d BaseLoop is nullptr\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop, const string& name, int socketFd, const InetAddress& localAddr, const InetAddress& peerAddr)
:_loop(checkLoopNotNull(loop))
, _name(name)
, _state(kConnecting)
, _reading(true)
, _socket(new Socket(socketFd))
, _channel(new Channel(loop, socketFd))
, _localAddr(localAddr)
, _peerAddr(peerAddr)
, _highWaterMark(64*1024*1024)  //64m
{ 
    //给channel设置相应的回调，poller监听这些事件 
    _channel->setReadCallback(bind(&TcpConnection::handleRead, this, placeholders::_1));
    _channel->setWriteCallback(bind(&TcpConnection::handleWrite, this));
    _channel->setCloseCallback(bind(&TcpConnection::handleClose, this));
    _channel->setErrorCallback(bind(&TcpConnection::handleError, this));
    LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n", _name.c_str(), _socket->fd());
    _socket->setKeepAlive(true);
}
TcpConnection::~TcpConnection(){
    LOG_INFO("~TcpConnection::dtor[%s] at fd = %d state = %d \n", name().c_str(), _channel->fd(), (int)_state);
}

void TcpConnection::send(const string &buf){
    if(_state == kConnected){
        if(_loop->isInLoopThread()){
            sendInLoop(buf.c_str(),buf.size());
        }else{
            _loop->runInLoop(bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}
//实际的发送数据的函数，应该写入缓冲区再发送，防止速度上的不匹配
void TcpConnection::sendInLoop(const void* data, size_t len){
    ssize_t nowWrite = 0; //  已经往缓冲器写的数据量
    ssize_t remaining = len; //剩余还没有往缓冲区写的数据量
    bool faultError = false;
    if (_state == kDisconnected)
    {
        LOG_ERROR("disconnected, give up writing!");
        return;
    }
    //第一次往channel里写数据，缓冲区里没有待发送数据
    if(_channel->isWriting() && _outputBuffer.readableBytes() == 0){
        nowWrite = ::write(_channel->fd(), data, len);
        if(nowWrite >= 0){
            remaining = len - nowWrite;
            //一次就把数据发送完了
            if(remaining == 0 && _writeCompleteCallback){
                //执行数据发送完成的回调
                _loop->queueInLoop(bind(_writeCompleteCallback, shared_from_this()));
            }
        }else{
            //出错了
            nowWrite = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) // SIGPIPE  RESET
                {
                    faultError = true;
                }
            }           
        }
    }

    //之前的调用，并没有把数据全部发送出去，
    // 说明当前这一次write，并没有把数据全部发送出去，剩余的数据需要保存到缓冲区当中，然后给channel
    // 注册epollout事件，poller发现tcp的发送缓冲区有空间，会通知相应的sock-channel，调用writeCallback_回调方法
    // 也就是调用TcpConnection::handleWrite方法，把发送缓冲区中的数据全部发送完成
    if (!faultError && remaining > 0) 
    {
        // 目前发送缓冲区剩余的待发送数据的长度
        size_t oldLen = _outputBuffer.readableBytes();
        if (oldLen + remaining >= _highWaterMark
            && oldLen < _highWaterMark
            && _highWaterMarkCallback)
        {
            _loop->queueInLoop(
                bind(_highWaterMarkCallback, shared_from_this(), oldLen+remaining)
            );
        }
        _outputBuffer.append((char*)data + nowWrite, remaining);
        if (!_channel->isWriting())
        {
            _channel->enableWriting(); // 这里一定要注册channel的写事件，否则poller不会给channel通知epollout
        }
    }

}
//关闭连接
void TcpConnection::shutdown(){
    if(_state == kConnected){
        setState(kDisconnecting);
        _loop->runInLoop(bind(&TcpConnection::shutdownInLoop, this));

    }
}
void TcpConnection::shutdownInLoop(){
    if(!_channel->isWriting()){//outPutBuffer中的数据全部发送完成
        _socket->shutdownWrite();
    }
}

void TcpConnection::connectEstablished(){
    setState(kConnected);
    _channel->tie(shared_from_this());
    _channel->enableReading();  //poller注册channel的epollin事件
    _connectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed(){
    if(_state = kConnected){
        setState(kDisconnected);
        _channel->disableAll();
        _connectionCallback(shared_from_this());
    }
    _channel->remove();
}

void TcpConnection::handleRead(TimeStamp receiveTime){
    int savaErro = 0;
    ssize_t n = _inputBuffer.readFd(_channel->fd(), &savaErro);
    if(n > 0){
        _messageCallback(shared_from_this() , &_inputBuffer, receiveTime);
    }else if(n == 0){
        handleClose();
    }else{
        errno = savaErro;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite(){
    if(_channel->isWriting()){
        int saveErro = 0;
        ssize_t n = _outputBuffer.writeFd(_channel->fd(), &saveErro);
        if(n > 0){
            _outputBuffer.retrive(n);
            if(_outputBuffer.readableBytes() == 0){
                _channel->disableWriting();
                if(_writeCompleteCallback){
                    _loop->queueInLoop(bind(_writeCompleteCallback, shared_from_this()));
                }
            }
            if(_state == kDisconnecting){
                shutdownInLoop();
            }
        }else{
            LOG_ERROR("TcpConnection::handleWrite");
        }    
    }else{
        LOG_ERROR("TcpConnection fd=%d is down, no more writing \n", _channel->fd());
    }
}
//给channel的方法，链接关闭时调用
void TcpConnection::handleClose(){
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d \n", _channel->fd(), (int)_state);
    setState(kDisconnected);
    _channel->disableAll();
    TcpConnectionPtr connPtr(shared_from_this());
    _connectionCallback(connPtr);
    _closeCallback(connPtr); //TCPServer给的，用户设立的回调方法
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(_channel->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n", _name.c_str(), err);
}