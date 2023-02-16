#pragma once
#include "noncopyable.hpp"
#include <iostream>
#include <string>
#include "TimeStamp.hpp"
using namespace std;
enum LogLevel{
    INFO,
    ERROR,
    FATAL,
    DEBUG,
};

//在这里设置宏定义，让用户更方便的使用日志模块
#define LOG_INFO(logmsg, ...) \
    do \
    {  \
        Logger& logger = Logger::getInstance();\
        logger.setLogLevel(INFO);\
        char buf[1024] = {0};\
        snprintf(buf, 1024, logmsg, ##__VA_ARGS__);\
        logger.log(buf);\
    } while (0)

#define LOG_ERROR(logmsg, ...) \
    do \
    {  \
        Logger& logger = Logger::getInstance();\
        logger.setLogLevel(ERROR);\
        char buf[1024] = {0};\
        snprintf(buf, 1024, logmsg, ##__VA_ARGS__);\
        logger.log(buf);\
    } while (0)

#define LOG_FATAL(logmsg, ...) \
    do \
    {  \
        Logger& logger = Logger::getInstance();\
        logger.setLogLevel(FATAL);\
        char buf[1024] = {0};\
        snprintf(buf, 1024, logmsg, ##__VA_ARGS__);\
        logger.log(buf);\
        exit(-1); \
    } while (0)

#ifdef MUDUODEBUG
#define LOG_DEBUG(logmsg, ...) \
    do \
    {  \
        Logger& logger = Logger::getInstance();\
        logger.setLogLevel(DEBUG)\
        char buf[1024];\
        sprintf(buf, 1024, logmsg, ##__VA_ARGS__);\
        logger.log(buf);\
    } while (0)
#else
    #define LOG_DEBUG(logmsg, ...) 
#endif



class Logger:Noncopyable{
public:
    static Logger& getInstance();
    void setLogLevel(int level);
    void log(string msg);
    
private:
    Logger(){}
    int _logLevel;

};