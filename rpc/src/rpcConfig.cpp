#include"../include/rpcConfig.h"

void Config::LoadConfigFile(const char *config_path){
FILE *p = fopen(config_path, "r");
if(NULL == p){
std::cout << "failed to open file:" << config_path << std::endl;
exit(EXIT_FAILURE);
}
//接下来要处理文件内容
while(!feof(p)){
char buf[512] = {0};
fgets(buf, 512, p);

std::string read_buf(buf);
Trim(read_buf);

// 判断#的注释
if (read_buf[0] == '#' || read_buf.empty())
{
    continue;
}

// 解析配置项
int idx = read_buf.find('=');
if (idx == -1)
{
    // 配置项不合法
    continue;
}

std::string key;
std::string value;
key = read_buf.substr(0, idx);
Trim(key);
// rpcserverip=127.0.0.1\n
int endidx = read_buf.find('\n', idx);
value = read_buf.substr(idx+1, endidx-idx-1);
Trim(value);
m_configMap.insert({key, value});
}
fclose(p);

}

//查询某项配置信息
std::string Config::Load(const std::string &key){
    auto it = m_configMap.find(key);
    if(it == m_configMap.end()){
        return "";
    }
    return it->second;
}

void Config::Trim(std::string &buf){
    int idx = buf.find_first_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串前面有空格
        buf = buf.substr(idx, buf.size()-idx);
    }
    // 去掉字符串后面多余的空格
    idx = buf.find_last_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串后面有空格
        buf = buf.substr(0, idx+1);
    }
}