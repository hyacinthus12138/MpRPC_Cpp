/*

其实就是对epoll使用， 主要是epoll create wait ctl 的封装 
值得一提的是 poller 和 channel 并没有接口直接通信
两者是通过eventloop进行通信的

*/
#pragma once

#include"Poller.h"

#include<vector>
#include<sys/epoll.h>


class EPollPoller : public Poller{

public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;

    /*
     重写基类Poller的抽象方法
    关于poll函数， epoll wait 就是在这里实现， eventloop会调用loop函数
    activechannels是一个传入参数， epoll会把有读写时间的socket写入activechannels，
    来通知event loop事件循环
    */
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    int epollfd_;

    // 填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    // 更新channel通道
    void update(int operation, Channel *channel);

    /*
    记录接收到的读写事件
    这个eventlist 被用在epoll wait 的第二个参数
    是一个传入参数， epoll会把监听到的event写入到这个events_中
    使用vector， 可以动态扩容, 使用kiniteventlistsize->16 初始化
    */
    using EventList = std::vector<epoll_event>;
    EventList events_;
    static const int kInitEventListSize = 16;


};