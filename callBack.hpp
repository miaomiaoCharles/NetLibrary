//callBack用于存储回调函数，就是用户设置的那些回调函数，包括当有新链接时候该执行什么，有新消息时候该执行什么之类的
#pragma once

#include <memory>
#include <functional>
#include "TimeStamp.hpp"
using namespace std;
class Buffer;
class TcpConnection;
using TcpConnectionPtr = shared_ptr<TcpConnection>;

using ConnectionCallBack = function<void(const TcpConnectionPtr&)>;
using CloseCallBack = function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallBack = function<void(const TcpConnectionPtr&)>;
using MessageCallBack = function<void(const TcpConnectionPtr&, Buffer*, TimeStamp)>;
using HighWaterMarkCallBack = function<void(const TcpConnectionPtr&, size_t)>; //防止发送或接收速率过快
