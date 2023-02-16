#include "Acceptor.hpp"

static int createNonblocking(){ //创建一个fd用于监听新用户连接用
    int fd = ::socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC, 0);
    if(fd < 0){
        LOG_FATAL("%s:%s:%d listen socket create err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return fd;
}  

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport) //传入的地址是自己的地址
:_loop(loop)
,_acceptSocket(createNonblocking())
,_acceptChannel(loop, _acceptSocket.fd())
,_listenning(false)
{
    _acceptSocket.setReuseAddr(true);
    _acceptSocket.setReusePort(true);
    _acceptSocket.bindAddress(listenAddr);
    _acceptChannel.setReadCallback(bind(&Acceptor::handleRead,this));
}
Acceptor::~Acceptor(){
    _acceptChannel.disableAll();
    _acceptChannel.remove();
}
void Acceptor::listen(){
    _listenning = true;
    _acceptSocket.listen(); //开启监听状态，这样的话当有新连接时候，baseLoop的wait()就会调用channel的回调方法
    _acceptChannel.enableReading();
}
void Acceptor::handleRead(){ //发生了新链接的事件，由baseloop控制channel调用这个回调操作
    InetAddress peerAddr;
    int connfd = _acceptSocket.accept(&peerAddr); //取出发生连接的fd，并把它的地址放入peerAddr中
    if (connfd >= 0) 
    {
        if (_newConnectionCallBack)
        {
            _newConnectionCallBack(connfd, peerAddr); // 轮询找到subLoop，唤醒，分发当前的新客户端的Channel
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE)
        {
            LOG_ERROR("%s:%s:%d sockfd reached limit! \n", __FILE__, __FUNCTION__, __LINE__);
        }
    }

}
