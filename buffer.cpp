#include "buffer.hpp"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

//buffer缓冲区是有大小的，但是从fd读取数据却不知道大小，也许就超过缓冲区大小了
ssize_t Buffer::readFd(int fd, int* saveErro){
    char extrabuf[65536] = {0}; //也就是说，fd上的数据最多不超过64k
    iovec vec[2]; //两个缓冲区，用readV来写，一个不够自动写第二个

    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writeableBytes();
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    ssize_t n = readv(fd, vec, 2);
    if(n < 0){
        *saveErro = errno;
    }else if(n < writeableBytes()){
        _writeIndex += n;
    }else{ //占用了栈数组
        _writeIndex = _buffer.size();
        append(extrabuf, n-writeableBytes());
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int* saveErro){
    ssize_t n = ::write(fd, peek(), readableBytes());
    if(n < 0){
        *saveErro = errno;
    }
    return n;
}