#include <iostream>
#include <string>
#include <vector>

#include "Friend.pb.h"
#include "MprpcApplication.h"
#include "RpcProvider.h"

class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendList(uint32_t id) 
    {
        std::cout << "Get friend list process" << std::endl;
        std::vector<std::string> friend_list;
        friend_list.push_back("friend1");
        friend_list.push_back("friend2");
        friend_list.push_back("friend3");
        return friend_list;
    }

    void GetFriendList(::google::protobuf::RpcController* controller,
                const ::fixbug::GetFriendListRequest* request,
                ::fixbug::GetFriendListResponse* response,
                ::google::protobuf::Closure* done)
    {   
        uint32_t id = request->userid();
        std::vector<std::string> friend_list = GetFriendList(id);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for (std::string &name : friend_list) {
            std::string *p = response->add_friends();
            *p = name;
        }

        // 执行回调函数，执行响应对象的数据序列化和网络发送（都是由框架完成）
        done->Run();
    }
private:

};

int main(int argc, char **argv) 
{   
    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    RpcProvider provider;
    provider.NotifyService(new FriendService());
    provider.Run();
    return 0;
}

