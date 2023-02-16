#include "tcpServer.hpp"
#include "Logger.hpp"
static EventLoop* checkLoopNotNull(EventLoop* loop){
    if(loop == nullptr){
        LOG_FATAL("%s:%s:%d BaseLoop is nullptr\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& name, Option option)
:_loop(checkLoopNotNull(loop))
, _ipPort(listenAddr.toIpPort())
, _name(name)
, _acceptor(new Acceptor(_loop,listenAddr, option == kReusePort))
, _threadPool(new EventLoopThreadPool(loop, name))
, _connectionCallBack() //这两个回调会有默认的，目前暂时空缺
, _messageCallBack()
, _nextConnId(1)
, _started(0)
{
    //当有新用户连接时，会执行newConnection回调
    _acceptor->setNewConnectionCallBack(bind(&TcpServer::newConnection, this, placeholders::_1, placeholders::_2));
}

TcpServer::~TcpServer()
{
    for (auto &item : _connections)
    {
        // 这个局部的shared_ptr智能指针对象，出右括号，可以自动释放new出来的TcpConnection对象资源了
        TcpConnectionPtr conn(item.second); 
        item.second.reset();

        // 销毁连接
        conn->getLoop()->runInLoop(
            bind(&TcpConnection::connectDestroyed, conn)
        );
    }
}

void TcpServer::setThreadNum(int num){
    _threadPool->setNumThread(num);
}

void TcpServer::start(){
    if(_started ++ == 0){ //这里就知道了为什么要把started设为int型，就是可以防止多次start
        _threadPool->start(_threadInitCallback);//该回调是用户在tcpserver里设置的
        _loop->runInLoop(bind(&Acceptor::listen, _acceptor.get()));
        //为什么不能直接_accptor->listen();
    }
}
void TcpServer::newConnection(int socketFd, const InetAddress& peerAddr){
    EventLoop* ioLoop = _threadPool->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", _ipPort.c_str(), _nextConnId);
    ++_nextConnId;
    string connName = _name + buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n",
        _name.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());
    // 通过sockfd获取其绑定的本机的ip地址和端口信息
    sockaddr_in local;
    ::bzero(&local, sizeof local);
    socklen_t addrlen = sizeof local;
    if (::getsockname(socketFd, (sockaddr*)&local, &addrlen) < 0)
    {
        LOG_ERROR("sockets::getLocalAddr");
    }
    InetAddress localAddr(local);
    // 根据连接成功的sockfd，创建TcpConnection连接对象
    TcpConnectionPtr connPtr(new TcpConnection(ioLoop,connName,socketFd,localAddr, peerAddr));
    _connections[connName] = connPtr;
    // 下面的回调都是用户设置给TcpServer=>TcpConnection=>Channel=>Poller=>notify channel调用回调
    connPtr->setConnectionCallback(_connectionCallBack);
    connPtr->setMessageCallback(_messageCallBack);
    connPtr->setWriteCompleteCallback(_writeCompleteCallBack);
    connPtr->setCloseCallback(bind(&TcpServer::removeConnection, this, placeholders::_1));
    // 直接调用TcpConnection::connectEstablished
    ioLoop->runInLoop(bind(&TcpConnection::connectEstablished, connPtr));    
}
void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    _loop->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n", 
        _name.c_str(), conn->name().c_str());

    _connections.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop(); 
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}