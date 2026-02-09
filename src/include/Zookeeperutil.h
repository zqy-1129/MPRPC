#pragma once

#include <string>
#include <zookeeper/zookeeper.h>
#include <semaphore.h>

// zkclient封装类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // zkclient连接服务器
    void Start();
    // zkclient在zkserver上创建znode节点
    void Create(const char* path, const char* data, int datalen, int state = 0);
    std::string GetData(const char* path);

private:  
    // zkclient句柄
    zhandle_t *m_zhandle;
};