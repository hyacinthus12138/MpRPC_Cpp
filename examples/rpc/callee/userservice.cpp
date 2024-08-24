#include<iostream>
#include<string>
#include"user.pb.h"
#include"application.h"
#include"provider.h"

/*
UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和Register
*/

class UserService : public fixbug::UserServiceRpc //在callee使用
{
public:
    bool Login(std::string name, std::string pwd){
        std::cout << "userService ==> local method: Login" << std::endl;
        std::cout << "name: " << name << " pwd: " << pwd << std::endl;
        return true; 
    }

    bool Register(uint32_t id, std::string name, std::string pwd){
        std::cout << "userService ==》 local method: Register" << std::endl;
        std::cout << "id:" << id << " name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }


    /*
    重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    1. caller   ===>   Login(LoginRequest)  => muduo =>   callee 
    2. callee   ===>    Login(LoginRequest)  => 交到下面重写的这个Login方法上了
    */
    void Login(google::protobuf::RpcController * controller,
                const fixbug::LoginRequest* request,
                fixbug::LoginResponse* response,
                google::protobuf::Closure* done){

        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name, pwd); 

        //把得到的结果写入response
        //这里假设调用成功了
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);

        //执行done回调， 这里的done回调绑定了sendRpcResponse,他会把结果返回给caller
        done->Run();
                
    }

    void Register(google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done){
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(id, name, pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_sucess(ret);

        done->Run();
    }

};

int main(int argc, char **argv){
    rpcApplication::Init(argc, argv);

    provider test_provider;
    test_provider.notifyService(new UserService());
    test_provider.run();

    return 0;
}