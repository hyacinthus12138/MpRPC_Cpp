#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include<string>


//封装socket地址类型， ip and port
//其实就是对sockaddr_in的使用进行了封装
class InetAddress{
public:

    //构造函数， 传入port和IP， 下面的是直接传入sockaddr
    explicit InetAddress(uint16_t port = 0,
            std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in &addr)
        :addr_(addr){};


    std::string toIP() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    //用来获取类的sockaddr_in
    const sockaddr_in* getSockAddr() const{return &addr_;};

    void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }

private:
    sockaddr_in addr_;
};