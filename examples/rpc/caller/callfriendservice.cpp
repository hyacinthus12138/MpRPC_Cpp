#include<iostream>
#include"application.h"
#include"friend.pb.h"
#include"rpcChannel.h"


int main(int argc, char **argv){
    rpcApplication::Init(argc, argv);

    fixbug::FiendServiceRpc_Stub stub(new rpcChannel());
    fixbug::GetFriendsListRequest request;
    request.set_userid(1000);

    fixbug::GetFriendsListResponse response;

    stub.GetFriendsList(nullptr, &request, &response, nullptr);
    if (0 == response.result().errcode())
    {
        std::cout << "Friendservice GetFriendsList response success!" << std::endl;
        int size = response.friends_size();
        for (int i=0; i < size; ++i)
        {
            std::cout << "index:" << (i+1) << " name:" << response.friends(i) << std::endl;
        }
    }
    else
    {
        std::cout << "FriendService GetFriendsList response error : " << response.result().errmsg() << std::endl;
    }

    return 0;
}