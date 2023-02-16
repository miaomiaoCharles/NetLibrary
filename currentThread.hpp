#pragma once
#include <unistd.h>
#include <sys/syscall.h>
//主要功能：封装了系统获取线程id的方法，同时，提供缓存机制，不会多次调用系统接口获取id
namespace currentThread{
    extern __thread int t_cachedTid; //全局变量放在.data，所有的线程都用的同一个，__thread的作用就是复制到每一个线程里
    void cacheTid();
    inline int tid(){
        if(__builtin_expect(t_cachedTid ==0, 0)){//t_cacheTid为0，说明之前没获取过
            cacheTid();
        }
        return t_cachedTid;
    }
}