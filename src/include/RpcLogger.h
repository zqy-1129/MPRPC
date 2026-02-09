#pragma once
#include "LockQueue.h"
#include <string>

enum LogLevel
{
    INFO, 
    ERROR,
};

class RpcLogger
{
public:
    RpcLogger(const RpcLogger &) = delete;
    RpcLogger(RpcLogger&&) = delete;
    bool operator=(const RpcLogger &) = delete;

    // 获取实例
    static RpcLogger& GetInstance();

    // 设置日志级别
    void SetLogLevel(LogLevel level);
    // 写日志
    void Log(std::string msg);
private:
    RpcLogger();
    int m_loglevel;  // 日志级别
    LockQueue<std::string> m_lockQueue;  // 日志缓冲队列
};

// 注意这里定义宏的之后之后需要加一个空行
#define LOG_INFO(logmsgformat, ...)  \
do  \
{   \
    RpcLogger &logger = RpcLogger::GetInstance();  \
    logger.SetLogLevel(INFO);  \
    char c[1024] = {0};  \
    snprintf(c, 1024, logmsgformat, ##__VA_ARGS__);  \
    logger.Log(c); \
} while(0)

#define LOG_ERROR(logmsgformat, ...)  \
do  \
{   \
    RpcLogger &logger = RpcLogger::GetInstance();  \
    logger.SetLogLevel(ERROR);  \
    char c[1024] = {0};  \
    snprintf(c, 1024, logmsgformat, ##__VA_ARGS__);  \
    logger.Log(c); \
} while(0)

