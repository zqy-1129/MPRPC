#include "Zookeeperutil.h"
#include "MprpcApplication.h"
#include <iostream> 

// 全局的watcher函数， zkclient在连接zkserver，zkserver发生变化时会调用这个函数 
void global_watecher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)  // 连接事件
    {
        if (state == ZOO_CONNECTED_STATE)  // 连接成功
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);  // 获取信号量对象
            sem_post(sem);  // 发送连接成功的信号，解除阻塞（唤醒sem_wait）
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{

}  

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {   
        zookeeper_close(m_zhandle);  // 关闭句柄，释放资源
    }
}

void ZkClient::Start()
{
    // 从配置文件中读取zkserver的ip和端口号
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    /**
     * zookeeper_mt: 多线程版本的zkclient， 线程安全的
     * zookeeper_st: 单线程版本的zkclient， 线程不安全的
     * zookeeper的API客户端程序提供了三个线程
     * API线程：专门负责调用zookeeper API的线程(zookeeper_init)， 由用户程序调用zookeeper_init函数创建
     * 网络I/O线程：专门负责网络I/O的线程 pthread_create poll
     * 事件回调线程：专门负责事件回调的线程 pthread_create
     */

    // 连接zkserver (host:port, 回调函数watcher， 超时时间，clientid，context，flags)
    m_zhandle = zookeeper_init(connstr.c_str(), global_watecher, 30000, nullptr, nullptr, 0);
    if (m_zhandle == nullptr)
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0); 
    zoo_set_context(m_zhandle, &sem);  // 将信号量对象传递给zookeeper客户端句柄（因为该信号量通知的线程和本线程不是一个）
    sem_wait(&sem);  // 阻塞等待连接成功的信号
    std::cout << "zookeeper_init success!" << std::endl;
}

void ZkClient::Create(const char* path, const char* data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (flag == ZNONODE)  // znode不存在
    {   
        // state: 0-持久化节点， ZOO_EPHEMERAL-临时节点， ZOO_SEQUENCE-顺序节点（使用get查看ephemeralOwner=0x0是永久的 ）
        flag = zoo_create(m_zhandle, path, data, datalen, 
            &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if (flag == ZOK)
        {
            std::cout << "znode create success! path:" << path << std::endl;
        }
        else
        {   
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error! path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // 创建znode节点
    int ret = zoo_create(m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, flag, path_buffer, bufferlen);
    if (ret == ZOK)
    {
        std::cout << "znode create success! path:" << path << std::endl;
    }
    else if (ret == ZNODEEXISTS)
    {
        std::cout << "znode already exists! path:" << path << std::endl;
    }
    else
    {
        std::cout << "znode create error! path:" << path << std::endl;
        exit(EXIT_FAILURE);
    }
}

// 根据path获取znode节点的数据
std::string ZkClient::GetData(const char* path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    // 实际就是调用get取获取节点数据， 但是如果节点不存在会返回错误码ZNONODE
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if (flag != ZOK)
    {
        std::cout << "znode get error! path:" << path << std::endl;
        return "";
    }
    return buffer;
}

