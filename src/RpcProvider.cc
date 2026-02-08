#include "RpcProvider.h"
#include "MprpcApplication.h"
#include "RpcHeader.pb.h"

/**
 * service_name -> service描述
 *                      -> service* 记录服务对象
 *                      method_name -> method方法对象
 */

void RpcProvider::NotifyService(google::protobuf::Service *service)
{   
    ServiceInfo service_info;
    // 获取服务对象的描述指针
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象服务的方法的数量
    int methodCnt = pserviceDesc->method_count();

    std::cout << "service name: " << service_name << std::endl;

    for (int i = 0; i < methodCnt; i++) 
    {   
        // 获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        std::cout << "method name: " << method_name << std::endl;
    }

    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// 服务的启动
void RpcProvider::Run()
{
    // 服务器的ip和端口号
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);
    // 创建TcpServer
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // 绑定连接回调和消息读写的回调方法
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, 
        std::placeholders::_2, std::placeholders::_3));
    // 设置muduo库的工作线程数量
    server.setThreadNum(4);

    std::cout << "RpcProvider start service at ip: " << ip << " " << port << std::endl;

    // 启动网络服务
    server.start();
    m_eventLoop.loop();
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (!conn->connected())
    {
        // 和rpc客户端的连接断开了
        conn->shutdown();
    }
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {
        // 序列化成功，通过网络把rpc方法执行的结果发送回到rpc的调用方
        conn->send(response_str);
        conn->shutdown();
    } 
    else 
    {
        std::cout << "Serialize response_str error!" << std::endl;
    }

}


/**
 *  框架内部，RpcProvider和RpcConsumer协商好通信用的protobuf数据类型
 *  service_name method_name args 定义proto的消息格式
 *  header_size + header_str + args_str (service_name method_name args_size)
 */

void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp timestamp)
{   
    // 网络上接受的远程rpc调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    // 将string中的内容，拷贝4个字节的内容到该位置
    recv_buf.copy((char*)&header_size, 4, 0);

    // 根据header_size读取数据头的原始字符流
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;   
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else 
    {
        // 数据头反序列化失败
        std::cout << "rpc_header_str: " << rpc_header_str << "parse error!" << std::endl; 
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "===========================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "===========================================" << std::endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << "is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << "is not exist!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service;  // 获取service对象 (new UserService)
    const google::protobuf::MethodDescriptor *method = mit->second;  // 获取method对象 (Login)

    // 生成rpc方法调用的请求request和response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request pasre error! content:" << args_str << std::endl;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用，绑定一个closeure的回调函数(也就是真正执行的函数)
    google::protobuf::Closure *done = 
        google::protobuf::NewCallback<RpcProvider, 
            const muduo::net::TcpConnectionPtr&, google::protobuf::Message*>
                (this, &RpcProvider::SendRpcResponse, conn, response);

    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}