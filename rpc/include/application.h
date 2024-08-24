#pragma once
#include"rpcConfig.h"
#include <unistd.h>

#include <iostream>
#include <string>


class rpcApplication{
public:
    static void Init(int argc, char **argv);
    static rpcApplication& getInstance();
    static Config& getConfig();
private:
    static Config m_config;

    rpcApplication(){};
    rpcApplication(const rpcApplication&) = delete;
    rpcApplication(rpcApplication&&) = delete;
};