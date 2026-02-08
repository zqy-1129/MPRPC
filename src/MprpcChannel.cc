#include "MprpcChannel.h"
#include "RpcHeader.pb.h"
#include "MprpcApplication.h"

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <error.h>
#include <unistd.h>

/**
 * header_size + service_name method_name + args
 */

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller, 
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response, 
                    google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();     // service_name
    std::string method_name = method->name();  // method_name
    uint32_t args_size = 0;
    std::string args_str;
    // 将request序列化
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    } 
    else{
        controller->SetFailed("serialize request error!");
        return;
    }

    // 定义rpc请求的header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else 
    {
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    // 组织待发送rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));  // header_size
    send_rpc_str += rpc_header_str;  // rpc_header
    send_rpc_str += args_str;

    // 打印调试信息
    std::cout << "===========================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "===========================================" << std::endl;

    // 使用tcp编程，完成rpc方法的远程调用
    // 创建套接字
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1) 
    {   
        controller->SetFailed("create socket error! errno: " + std::to_string(errno));
        return;
    }

    // 读取配置文件中的ip和port
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    // 设置服务器
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接到服务器
    if (connect(clientfd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {   
        controller->SetFailed("connect error! errno: " + std::to_string(errno));
        close(clientfd);
        return;
    }

    // 发送rpc请求（阻塞的）
    if (send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0) == -1)
    {
        controller->SetFailed("send error! errno: " + std::to_string(errno));
        close(clientfd);
        return;
    }

    // 接受rpc请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if ((recv_size = recv(clientfd, recv_buf, 1024, 0)) == -1)
    {
        controller->SetFailed("recv error! errno: " + std::to_string(errno));
        close(clientfd);
        return;
    }

    // 反序列化收到的响应
    // std::string response_str(recv_buf, 0, recv_size);   // recv_buf遇到'\0'就结束
    // if (!response->ParseFromString(response_str))
    if (!response->ParseFromArray(recv_buf, recv_size))
    {   
        controller->SetFailed("parse error! response_str: " + std::string(recv_buf, 0, recv_size));
        close(clientfd); 
        return;
    }
}