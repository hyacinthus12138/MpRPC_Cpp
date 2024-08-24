set(SRC_LIST  
    userservice.cpp
    ../userproto/user.pb.cc)
add_executable(user_provider ${SRC_LIST})
target_link_libraries(user_provider network protobuf rpc)

set(SRC
    friendservice.cpp 
    ../friendproto/friend.pb.cc)
add_executable(friend_provider ${SRC})
target_link_libraries(friend_provider network protobuf rpc)