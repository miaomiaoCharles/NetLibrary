#pragma once
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

class Buffer{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    explicit Buffer(size_t initialSize = kInitialSize)
    : _buffer(kCheapPrepend+initialSize)
    , _readIndex(kCheapPrepend)
    , _writeIndex(kCheapPrepend)
    {
    }

    size_t readableBytes() const{
        return _writeIndex - _readIndex;
    }
    size_t writeableBytes() const{
        return _buffer.size() - _writeIndex;
    }
    size_t prependableBytes() const{
        return _readIndex;
    }
    //读取了数据之后的复位操作
    void retrive(int len){
        if(len < readableBytes()){
            _readIndex += len;
        }else{
            retriveAll();
        }
    }
    void retriveAll(){
        _readIndex = _writeIndex = kCheapPrepend;
    }

    //返回缓冲区中可读数据的起始地址
    const char* peek()const{
        return begin() + prependableBytes();
    }
    //将可读缓冲区的数据全拿出来
    string retriveAllString(){
        return retriveAsString(readableBytes());
    }
    string retriveAsString(size_t len){
        string result(peek(), len);
        retrive(len);
        return result;
    }
    void ensureWriteableBytes(size_t len){
        if(writeableBytes() < len){
            makeSpace(len);
        }
    }
    char* beginWrite(){
        return begin() + _writeIndex;
    }
    //把[data, data+len]地址的数据写到write缓冲区当中
    void append(const char* data, size_t len){
        ensureWriteableBytes(len);
        copy(data, data+len, beginWrite());
        _writeIndex += len;
    }
    //把fd的内容写入缓冲区中，这里的read指的是读fd
    ssize_t readFd(int fd, int* saveErro); 
    //从缓冲区的读数据区拿数据写入fd
    ssize_t writeFd(int fd, int* saveErro);
private:
    char* begin(){
        return &*_buffer.begin();
    }
    const char* begin()const {
        return &*_buffer.begin();
    }
    //扩容操作
    void makeSpace(int len){
        //prependableBytes() + writeableBytes()是前面的空闲区域加后面的可写区域
        if(kCheapPrepend + len > prependableBytes() + writeableBytes()){
            _buffer.resize(_writeIndex + len);
        }else{
            //把reable缓冲器空下来的空间给write缓冲区，同时把未读的read数据往前移
            copy(begin()+ _readIndex, begin() + _writeIndex, begin() + kCheapPrepend);
            _readIndex = kCheapPrepend;
            _writeIndex = _readIndex + readableBytes();
        }
    }
    vector<char> _buffer;
    size_t _readIndex;
    size_t _writeIndex;
}; 