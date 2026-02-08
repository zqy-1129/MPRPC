#include <iostream>
#include <string>

#include "User.pb.h"
#include "MprpcApplication.h"
#include "RpcProvider.h"

/**
 * UserService原来是一个本地服务，
 * 两个本地方法Login和GetFriendLists
 */
class UserService : public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd) 
    {
        std::cout << "Login process" << std::endl;
        std::cout << "name: " << name << " password: " << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd) 
    {
        std::cout << "Register process" << std::endl;
        std::cout << "id: " << id << " name: " << name << " password: " << pwd << std::endl;
        return true;
    }

    /**
     * 重写基类UserServiceRpc的虚函数，下面的方法是给框架进行调用的
     * caller -> Login(LoginRequest) -> muduo -> callee
     * callee -> Login(LoginRequest) -> 交给这个重写的函数 -> 在该函数中调用对应的函数进行处理
     */ 
    void Login(::google::protobuf::RpcController* controller,
                const ::fixbug::LoginRequest* request,
                ::fixbug::LoginResponse* response,
                ::google::protobuf::Closure* done)
    {   
        /** 
         * 四个步骤：
         * 1. 得到请求参数
         * 2. 执行请求函数
         * 3. 响应进行设置
         * 4. 执行回调函数
         */

        // 框架给业务上报了请求参数LoginRequest，应用获取对应的数据处理
        // 这里是直接将数据以对象的形式进行操作，不用像JSON一样进行解析
        std::string name = request->name();
        std::string pwd = request->pwd();
        // 本地业务的实际调用
        bool login_result = Login(name, pwd);
        // 响应
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        // 执行回调函数，执行响应对象的数据序列化和网络发送（都是由框架完成）
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                const ::fixbug::RegisterRequest* request,
                ::fixbug::RegisterResponse* response,
                ::google::protobuf::Closure* done)
    {   
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        // 本地业务的实际调用
        bool Register_result = Register(id, name, pwd);
        // 响应
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(Register_result);

        // 执行回调函数，执行响应对象的数据序列化和网络发送（都是由框架完成）
        done->Run();
    }
private:

};

int main(int argc, char **argv) 
{   
    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());
    provider.Run();
    return 0;
}

