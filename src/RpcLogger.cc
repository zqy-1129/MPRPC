#include "RpcLogger.h"
#include <iostream>
#include <thread>

RpcLogger& RpcLogger::GetInstance()
{
    static RpcLogger RpcLogger;
    return RpcLogger;
}

RpcLogger::RpcLogger()
{
    // 启动日志线程
    std::thread writrLogTask([&](){
        for (;;)
        {
            // 获取当天提起，然后取日志信息，写入相应的日志文件当中 a+
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", 
                nowtm->tm_year + 1900,
                nowtm->tm_mon + 1,
                nowtm->tm_mday);
            
            FILE *pf = fopen(file_name, "a+");
            if (pf == nullptr)
            {
                std::cout << "logger file: " << file_name << " open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lockQueue.Pop();
            msg += "\n";
            char time_buf[128] = {0};
            sprintf(time_buf, "%d-%d-%d [%s] ", 
                nowtm->tm_hour, 
                nowtm->tm_min, 
                nowtm->tm_sec, 
                m_loglevel == INFO ? "INFO" : "ERROR");
            msg.insert(0, time_buf);
            fputs(msg.c_str(), pf);
            fclose(pf);
        }
    });
    writrLogTask.detach();
}

void RpcLogger::SetLogLevel(LogLevel level)
{
    m_loglevel = level;
}

void RpcLogger::Log(std::string msg)
{
    m_lockQueue.Push(msg);
}
