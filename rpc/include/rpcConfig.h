#pragma once

#include<string>
#include<iostream>
#include<unordered_map>

//里面记录的是
//serverip， port,  zookeeperip, port
//application会加载上面的四项

class Config{
public:

    //从文件中加载对应的配置
    void LoadConfigFile(const char* config_path);

    //查询功能
    std::string Load(const std::string &key);

private:
    std::unordered_map<std::string, std::string> m_configMap;
    
    //读取config文件的每行数据， 去除空格
    void Trim(std::string &buf);
};