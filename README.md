# NetLibrary
这是一个模仿muduo网络库实现的基于epoll的网络库框架。

想比起传统muduo，**该网络库具有以下优点**：

1. 传统muduo网络库需要包含大量的包，使用需添加很多头文件。本网络库使用时只需要添加一个头文件。
2. 传统muduo网络库需要依赖boost库，本网络库不需要提前安装boost库，相关功能全部使用C++11的代码重写。
3. 安装方便，只需要执行脚本即可自动安装。

**使用说明**：

在工作目录下，输入命令行：.

```bash
sudo ./autobuild.sh
```

之后会自动执行脚本，完成编译，并将生成的动态库放入/usr/lib下，头文件则拷贝到/usr/include/mymuduo下。

使用可以参考example/test.cpp中的用例。

**原理简介**：

采用非阻塞+I/O复用的Reactor模型，下图展示了如何接受新的连接的过程。

![image](https://github.com/miaomiaoCharles/myRpcMuduo/blob/main/%E6%96%B0%E8%BF%9E%E6%8E%A5%E5%8E%9F%E7%90%86%E5%B1%95%E7%A4%BA%E5%9B%BE.png)
