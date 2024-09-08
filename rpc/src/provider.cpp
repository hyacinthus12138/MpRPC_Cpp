#include"provider.h"
#include "zookeeperutil.h"



void provider::notifyService(google::protobuf::Service *service){
    serviceInfo service_info;

    const google::protobuf::ServiceDescriptor * sp = service->GetDescriptor();
    std::string serviceName = sp->name();
   
    int methodNum = sp->method_count();
   
    std::cout << "service_name:" << serviceName << std::endl;
    std::cout << "service_method_num:" << methodNum << std::endl; 
    for(int i = 0; i < methodNum; ++i){
        const google::protobuf::MethodDescriptor *mp = sp->method(i);
        std::string methodName = mp->name();
        service_info.m_methodMap.insert({methodName, mp});
        
        std::cout << "service:" << serviceName << " service method->" << i << " :" 
                            << methodName  << std::endl;
    }
    service_info.m_service = service;
    m_serviceMap.insert({serviceName, service_info});
}

void provider::run(){
    std::string ip = rpcApplication::getInstance().getConfig().Load("rpcserverip");
    uint16_t port = atoi(rpcApplication::getInstance().getConfig().Load("rpcserverport").c_str());
    InetAddress address(port, ip);

    TcpServer server(&m_eventloop, address, "rpcprovider");
    server.setConnectionCallback(std::bind(
        &provider::onConnection, this, std::placeholders::_1
    ));

    server.setMessageCallback(std::bind(
        &provider::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3
    ));

    server.setThreadNum(2);

    
    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久性节点    method_name为临时性节点
    for (auto &sp : m_serviceMap) 
    {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name   /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }
    
    std::cout << "rpcProvider start loop at ip:" << ip << " port:" << port << std::endl;
    server.start();
    m_eventloop.loop();
}


//muduo的cb
void provider::onConnection(const TcpConnectionPtr& conn){
    if(!conn->connected()){
        conn->shutdown();
    }
}

/*
    header_size(4字节) + header_str + args_str
                            |
            service_name + method_name + args_size
*/
void provider::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp){
    //使用muduo接收数据
    std::string recvBuf = buffer->retrieveAllAsString();

    //读取前4个字节 ==》header-size
    uint32_t header_size = 0;
    recvBuf.copy((char*)&header_size, 4, 0);

    //获取header_str, 解析对应的 service method
    std::string header_str = recvBuf.substr(4, header_size);
    wxz_rpc::header rpcheader;
    std::string service_name, method_name;
    uint32_t args_size;

    if(rpcheader.ParseFromString(header_str)){
        service_name = rpcheader.service_name();
        method_name = rpcheader.method_name();
        args_size = rpcheader.args_size();
    }
    else{
        std::cout << "header_str:" << header_str <<  " parsefromString failed " << std::endl;
        return;
    }

    //获取对应的 args_Str
    std::string args_str = recvBuf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << " in file provider" << std::endl;
    std::cout << "header_size:  " << header_size << std::endl; 
    std::cout << "header_str:   " << header_str << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name:  " << method_name << std::endl; 
    std::cout << "args_size:    " << args_size << std:: endl;
    std::cout << "args_str:     " << args_str << std::endl; 
    std::cout << "============================================" << std::endl;

    //获得 对应的 service 和 method 的指针
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end()){
        std::cout << service_name << " not exist" << std::endl;
        return;
    }
    auto mit = it->second.m_methodMap.find(method_name);
    if(mit == it->second.m_methodMap.end()){
        std::cout << service_name << " exist, and yet " << method_name << " not exist" << std::endl;
        return;
    }
    google::protobuf::Service *service = it->second.m_service;
    const google::protobuf::MethodDescriptor *method = mit->second; 

    //根据 args_str 生成 请求 request
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if(request->ParseFromString(args_str) == false){
        std::cout << "parse args_str:" << args_str << " failed" << std::endl;
        return;
    }

    //根据 service 和 method 获取 response 的数据格式， 在这里不进行填充
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    //  // 给下面的method方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure *done =
                     google::protobuf::NewCallback<provider,
                                                const TcpConnectionPtr&,
                                                google::protobuf::Message*>
                                                (this, 
                                                &provider::sendRpcResponse, 
                                                    conn, response);
    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)  
    //整个流程 muduo接受请求， --》 解析 service， method， args 
    //                                                      |
    //                                       调用下列函数， callmethod， 它会调用caller的Login
    //                                  也就是那四步， 解析request， 本地login函数， 填写 response
    //                                  最后调用done， 也就是 最终调用 sendrpcresponse                                            
    service->CallMethod(method, NULL, request, response, done);
}

// Closure的回调操作，用于序列化rpc的响应和网络发送
void provider::sendRpcResponse(const TcpConnectionPtr& conn, google::protobuf::Message *response){
    std::string response_str;
    if(response->SerializeToString(&response_str)){
        conn->send(response_str);
    }
    else {
        std::cout << " serialize response_str failed " << std::endl;
    }
    conn->shutdown();
    
}