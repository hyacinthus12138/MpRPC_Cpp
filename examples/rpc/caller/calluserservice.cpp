#include <iostream>
#include "application.h"
#include "user.pb.h"
#include "rpcChannel.h"

int main(int argc, char **argv){

    //框架初始化， 加载各种 ip 和 port
    rpcApplication::Init(argc, argv);

    //向 stub 传入 rpcchannel
    // rpcChannel 的 callMethod 被重写，rpcChannel 完成了 
    // 创建 request， 创建Tcp连接， 发送request， 阻塞等待， 接受response， 关闭Tcp连接
    fixbug::UserServiceRpc_Stub stub(new rpcChannel());

    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    fixbug::LoginResponse response;

    //这个方法就是 调用生成的UserServiceRpc_stub 的 Login 方法
    //这个Login 里面进而调用了 channel_->CallMethod(descriptor()->method(0),
    //                   controller, request, response, done);
    //也就是执行了 rpcChannel里面重写的callMethod方法
    stub.Login(NULL, &request, &response, NULL);

    if(0 == response.result().errcode()){
        std::cout << "caller Userservice Login response success:" << response.sucess() << std::endl; 
    }
    else{
        std::cout << "caller Userservice Login response failed:" << response.sucess() << std::endl;
    }

    //下面是对 register 方法的演示
    fixbug::RegisterRequest regis_request;
    regis_request.set_id(1415);
    regis_request.set_name("li si");
    regis_request.set_pwd("741852");

    fixbug::RegisterResponse regis_response;

    stub.Register(NULL, &regis_request, &regis_response, NULL);

    if(0 == regis_response.result().errcode()){
        std::cout << "caller Userservice Register response success:" << regis_response.sucess() << std::endl; 
    }
    else{
        std::cout << "caller Userservice Register response failed:" << regis_response.sucess() << std::endl;
    }
    return 0;
}