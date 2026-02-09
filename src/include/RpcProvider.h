#pragma once
#include "MprpcApplication.h"

#include <string>
#include <functional>
#include <unordered_map>
#include <muduo/net/TcpServer.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/Buffer.h>
#include <muduo/base/Timestamp.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

// 框架提供用于发布rpc服务的网络对象类
class RpcProvider
{
public:
    // 提供给外部使用的，可以发布rpc方法的函数接口（这里是protobuf的服务基类指针来接受服务这样可以接受任意的基于该基类派生的服务）
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务节点，提供rpc远程网络调用服务
    void Run();

private:
    // 事件循环
    muduo::net::EventLoop m_eventLoop;

    // 服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;   // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;     // 保存服务对象的所有方法
    };
    // 存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // 新Socket连接回调（这里都是muduo的回调）
    void OnConnection(const muduo::net::TcpConnectionPtr& conn);

    void OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp timestamp);
    // closure回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
}; 