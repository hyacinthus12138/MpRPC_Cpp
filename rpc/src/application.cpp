#include"../include/application.h"


Config rpcApplication::m_config;

void showArgsHelp(){
    std::cout << "format error: command -i <configfile>" << std::endl;
}
void rpcApplication::Init(int argc, char **argv){
    if(argc < 2){
        showArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            showArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            showArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    
    // 开始加载配置文件了 rpcserver_ip=  rpcserver_port   zookeeper_ip=  zookepper_port=
    m_config.LoadConfigFile(config_file.c_str());
  
    
}

rpcApplication& rpcApplication::getInstance(){
    static rpcApplication app;
    return app;

};
Config& rpcApplication::getConfig(){
    return m_config;
};

