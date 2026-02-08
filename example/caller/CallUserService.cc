#include <iostream>
#include "MprpcApplication.h"
#include "User.pb.h"
#include "MprpcChannel.h"

int main(int argc, char **argv)
{
    // 整个程序启动后，想使用mprpc框架来使用rpc服务调用，
    // 一定需要先调用框架初始化函数，只初始化一次
    MprpcApplication::Init(argc, argv);

    // 调用远程发布的rpc服务
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    // 请求
    fixbug::LoginRequest request;
    request.set_name("zqy");
    request.set_pwd("123456");
    // 响应
    fixbug::LoginResponse response;
    stub.Login(nullptr, &request, &response, nullptr);   // RpcChannel调用其的callMethod方法集中来做所有Rpc方法调用的参数序列化和网络发送
    
    // 一次rpc调用完成
    if (response.result().errcode() == 0)
    {
        std::cout << "rpc login response success: " << response.success() << std::endl;
    } 
    else
    {
        std::cout << "rpc login response failed: " << response.result().errcode() << std::endl;
    }

    fixbug::RegisterRequest register_request;
    register_request.set_id(2000);
    register_request.set_name("zqy");
    register_request.set_pwd("123456");
    fixbug::RegisterResponse register_response;
    stub.Register(nullptr, &register_request, &register_response, nullptr);
    if (register_response.result().errcode() == 0)
    {
        std::cout << "rpc register response success: " << register_response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc register response failed: " << register_response.result().errcode()
                    << " errmsg: " << register_response.result().errmsg() << std::endl;
    }

    return 0;
}