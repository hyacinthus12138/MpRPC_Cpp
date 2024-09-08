set(SRC_LIST 
    application.cpp 
    head.pb.cc 
    provider.cpp 
    rpcChannel.cpp 
    rpcConfig.cpp 
    rpccontroller.cpp 
    zookeeperutil.cpp
    rpclogger.cpp 
    )
add_library(rpc ${SRC_LIST})
target_link_libraries(rpc network )
target_link_libraries(rpc network zookeeper_mt)