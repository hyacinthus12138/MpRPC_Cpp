set(SRC_LIST
    calluserservice.cpp 
    ../userproto/user.pb.cc)
add_executable(user_caller ${SRC_LIST})
target_link_libraries(user_caller network protobuf rpc)

set(SRC
    callfriendservice.cpp 
    ../friendproto/friend.pb.cc)
add_executable(friend_caller ${SRC})
target_link_libraries(friend_caller network protobuf rpc)