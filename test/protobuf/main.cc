#include <iostream>
#include <string>
#include "test.pb.h"

using namespace fixbug;

int main()
{   
    // LoginResponse rsp;
    // ResponseCode* rc = rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("登录失败");

    GetFriendListsResponse rsp;
    ResponseCode* rc = rsp.mutable_result();
    rc->set_errcode(0);

    User *user1 = rsp.add_users();
    user1->set_name("哈基米");
    user1->set_age(10);
    user1->set_sex(User::MAN);

    std::cout << rsp.users_size() << std::endl;

    User *user2 = rsp.mutable_users(0);
    std::cout << user2->name() << " " << user2->age() << std::endl;
    return 0;
}

// int main()
// {   
//     // 序列化
//     LoginRequest req;
//     req.set_name("哈基米");
//     req.set_pwd("123456");

//     std::string send_str;
//     if (req.SerializeToString(&send_str))
//     {
//         std::cout << send_str << std::endl;
//     }

//     // 反序列化
//     LoginRequest reqB;
//     if (reqB.ParseFromString(send_str))
//     {
//         std::cout << reqB.name() << std::endl;
//         std::cout << reqB.pwd() << std::endl;
//     }
//     return 0;
// }