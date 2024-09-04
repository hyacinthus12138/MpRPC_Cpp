# MpRPC_Cpp

使用protobuf，自己实现的muduo库，实现rpc服务


## 编译&&安装

    硬性要求：linux环境，C++编译器支持C++11标准。

    编译&&安装：
        git clone git@github.com:hyacinthus12138/MpRPC_Cpp.git
        cd MpRPC_Cpp
        cd build
        cmake ..
        make 
## lib目录会有两个文件
      libnetwork.a 这是tinymuduo库文件
      librpc.a     这是rpc库文件
## examples简介
      /examples/muduo 这是tinymuduo的测试文件一个简单的回声服务器
      /examples/rpc/caller
      /examples/rpc/callee 这是rpc client端和caller端的代码
## bin文件夹
     产生friend_caller friend_provider

     先运行 provider    ./friend_provider -i test.conf
            运行成功会显示  
                service_name:FiendServiceRpc
                service_method_num:1
                service:FiendServiceRpc service method->0 :GetFriendsList
                rpcProvider start loop at ip:127.0.0.1 port:8000
                [INFO]2024/08/31 17:00:21:func = updateChannel -> fd = 7 events = 3 index = -1 
            这里显示发布的rpc名字为FiendServicdRpc, 总共一种方法，GetFriendsList 以及muduo库的日志输出
      再运行caller    ./friend_caller -i test.conf
            运行成功返回 
                  in file rpcChannel
                  header_size: 35
                  rpc_header_str: 
                  FiendServiceRpcGetFriendsList
                  service_name: FiendServiceRpc
                  method_name: GetFriendsList
                  ============================================
                  Friendservice GetFriendsList response success!
                  index:1 name:zhang san
                  index:2 name:zhang  si
                  index:3 name:zhang  wu
            此时provider端显示
                  ============================================
                  in file provider
                  header_size:  35
                  header_str:   
                  FiendServiceRpcGetFriendsList
                  service_name: FiendServiceRpc
                  method_name:  GetFriendsList
                  args_size:    3
                  ============================================
                  callee local FriendService GetFriendList service:1000
