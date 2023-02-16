#include <iostream>
#include <mymuduo/tcpServer.hpp>
#include <mymuduo/Logger.hpp>
#include <functional>
using namespace std;
class TestServer{
public:
    TestServer(EventLoop* loop, const InetAddress& addr, const string& name)
    :_server(loop, addr, name)
    ,_loop(loop)
    {
        _server.setConnectionCallBack(bind(&TestServer::onConnection,this, placeholders::_1));
        _server.setMessageCallBack(bind(&TestServer::onMessage,this, placeholders::_1, placeholders::_2, placeholders::_3));
        _server.setThreadNum(3);
    }
    void startServer(){
        _server.start();
    }
private:
    void onConnection(const TcpConnectionPtr& conn){
        if(conn->connected()){
            LOG_INFO("connection success: %s",conn->peerAddress().toIpPort().c_str());
        }else{
            LOG_INFO("connection fail: %s",conn->peerAddress().toIpPort().c_str());
        }
    }
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, TimeStamp time){
        string msg = buf->retriveAllString();
        conn->send(msg);
        conn->shutdown();
    }
    EventLoop* _loop;
    TcpServer _server;
};
int main(){
    EventLoop loop;
    InetAddress addr(8000);
    TestServer server(&loop, addr, "myServer");
    server.startServer();
    loop.loop();
    return 0;
}