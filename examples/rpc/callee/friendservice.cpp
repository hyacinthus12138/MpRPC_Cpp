#include <iostream>
#include <string>
#include "friend.pb.h"
#include "application.h"
#include "provider.h"
#include <vector>


class FriendService : public fixbug::FiendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid){
        std::cout << "callee local FriendService GetFriendList service:" << userid << std::endl;
        std::vector<std::string> res;
        res.push_back("zhang san");
        res.push_back("zhang  si");
        res.push_back("zhang  wu");
        return res;
    }

    void GetFriendsList(google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendsListRequest* request,
                       ::fixbug::GetFriendsListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->userid();

        std::vector<std::string>  friendList = GetFriendsList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(std::string &name : friendList){
            std::string *p = response->add_friends();
            *p = name;
        }
        done->Run();
    }
};

int main(int argc, char **argv){
    rpcApplication::Init(argc, argv);

    provider test_provider;
    test_provider.notifyService(new FriendService());

    test_provider.run();

    return 0;
}