#pragma once

#include "MprpcConfig.h"

// 框架初始化
class MprpcApplication
{
public:
    MprpcApplication(const MprpcApplication &application) = delete;
    MprpcApplication(MprpcApplication&& application) = delete;
    bool operator=(const MprpcApplication &application) = delete;
    
    // 调用框架的初始化操作（例如 provider -i config.conf）
    static void Init(int argc, char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();

private:
    MprpcApplication() {}

    static MprpcConfig m_config;
};