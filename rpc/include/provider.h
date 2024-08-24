#pragma once
#include "google/protobuf/service.h"
#include "rpcConfig.h"
#include "application.h"
#include "head.pb.h"
#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>
#include <iostream>
/*
    provider有两个主要方法
     
    notify 作用是 传入的 service 发布为 rpc 方法，本质上就是计入map中
    run 开启muduo库 监听事件处理callback


*/

class provider{

public:
    void notifyService(::google::protobuf::Service *service);

    void run();
private:

    EventLoop m_eventloop;

    //记录传入的一个rpcservice的基本信息
    //包括一个service和对应的method方法
    struct serviceInfo{
        google::protobuf::Service *m_service;//保存传入的service
        //map的string记录的是method的名字
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };

    //这里的string 记录的是传入的service的名字
    //serviceInfo里面会记录servcie所有method的名字和method指针
    std::unordered_map<std::string, serviceInfo> m_serviceMap;

    //muduo的cb
    void onConnection(const TcpConnectionPtr&);
    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp);

    // Closure的回调操作，用于序列化rpc的响应和网络发送
    void sendRpcResponse(const TcpConnectionPtr&, google::protobuf::Message*);

};