#pragma once
#include "rpclockqueue.h"
#include <string>

// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
#define rpcLOG_INFO(logmsgformat, ...) \
    do \
    {  \
        rpcLogger &logger = rpcLogger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

#define rpcLOG_ERR(logmsgformat, ...) \
    do \
    {  \
        rpcLogger &logger = rpcLogger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c); \
    } while(0) \

// 定义日志级别
enum LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
};

// Mprpc框架提供的日志系统
class rpcLogger
{
public:
    // 获取日志的单例
    static rpcLogger& GetInstance();
    // 设置日志级别 
    void SetLogLevel(LogLevel level);
    // 写日志
    void Log(std::string msg);
private:
    int m_loglevel; // 记录日志级别
    rpcLockQueue<std::string>  m_lckQue; // 日志缓冲队列

    rpcLogger();
    rpcLogger(const rpcLogger&) = delete;
    rpcLogger(rpcLogger&&) = delete;
};