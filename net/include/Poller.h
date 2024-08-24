#pragma once

#include"noncopyable.h"
#include"Timestamp.h"

#include<vector>
#include<unordered_map>

class Channel;
class EventLoop;


class Poller : noncopyable{

public:
    using ChannelList = std::vector<Channel*>;
    /*
    //这个ChannelList其实是维护在eventloop事件循环中的
    //poller中维护的是一个channelmap
    //前者是eventloop出现的所有socket， 后者是注册在epoll的所有socket
    */

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    // 给所有IO复用保留统一的接口

    //子类会重写这个poll， epoll就是在这实现运行
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0; 
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    //poll维护一个map， channels_, 
    //里面包含所有poller监听的Channel对象
    //这个函数判断是否poller有监听传入的channel
    bool hasChannel(Channel *channel) const;

    // EventLoop可以通过该接口获取默认的IO复用的具体实现
    //可以理解为父类指针可以获得子类创建的epoll或者poll对象
    //这里源码中没有在Poller.h中实现， 而是单独使用了一个
    //DefaultPoller.cpp, 原因可以看出， 这个函数调用了Poller的
    //子类的一些函数， 如果在Poller中实现， 会出现父类包含子类
    //头文件的情况
    static Poller* newDefaultPoller(EventLoop *loop);

protected:
    //map key : sockfd, value, 对应的Channel对象的指针
    //设置为protected， 子类由此可以得知poller监听的socket
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_; //每个poller都有一个eventloop来管理
    //构造函数传入的loop就是管理当下poller的eventloop

};