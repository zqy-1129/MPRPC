#include <iostream>
#include "MprpcApplication.h"
#include "Friend.pb.h"


int main(int argc, char **argv)
{
    // 整个程序启动后，想使用mprpc框架来使用rpc服务调用，
    // 一定需要先调用框架初始化函数，只初始化一次
    MprpcApplication::Init(argc, argv);

    // 调用远程发布的rpc服务
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());
    // 请求
    fixbug::GetFriendListRequest request;
    request.set_userid(1000);
    // 响应
    fixbug::GetFriendListResponse response;
    
    MprpcController controller;

    stub.GetFriendList(&controller, &request, &response, nullptr);   // RpcChannel调用其的callMethod方法集中来做所有Rpc方法调用的参数序列化和网络发送
    
    // 一次rpc调用完成
    if (controller.Failed())
    {
        std::cout << "rpc get friends failed! error: " << controller.ErrorText() << std::endl;
        return 0;
    } 
    else
    {
        if (response.result().errcode() == 0)
        {
            std::cout << "rpc get friends success" << std::endl;
            for (int i = 0; i < response.friends_size(); ++i)
            {
                std::cout << "friend name: " << response.friends(i) << std::endl;
            }
        } 
        else
        {
            std::cout << "rpc get friends failed: " << response.result().errcode() << std::endl;
        }
    }
    return 0;
}