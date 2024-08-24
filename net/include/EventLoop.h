/*
eventloop 类似于 reactor， ooller 类似于 事物分发器
eventloop 控制channel（sockfd） 到 poller（epoll）中， poller 通过 activeChannel 把监听到的事件返回到eventloop（reactor）
eventloop 通知 channel 调用 callback 处理对应事件
*/
#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"
#include"noncopyable.h"


class Channel;
class Poller;

class EventLoop : noncopyable{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    /*
    开启事件循环
    */
    void loop();

    /*
    退出事件循环
    */
    void quit();

    Timestamp pollReturnTime() const{ return pollReturnTime_; };

    /*
    前者会在当前loop中执行cb
    后者先把cb放入队列， 唤醒loop所在的线程， 执行cb

    */
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    // 用来唤醒loop所在的线程的
    void wakeup();

    // EventLoop的方法 =》 Poller的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    // 判断EventLoop对象是否在自己的线程里面
    bool isInLoopThread() const { return threadId_ ==  CurrentThread::tid(); }

private:

    /*
    wakeup 
    */
   void handleRead();

    /*
    执行回调
    */
   void doPendingFunctors();

   int createEventfd();

    //eventloop 通过activeChannels_来接受poller返回的活跃socket
    using ChannelList = std::vector<Channel*>;
    ChannelList activeChannels_;

    //定义为原子操作， looping_表示正在执行loop循环， quit_则是推出loop循环
    std::atomic_bool looping_;
    std::atomic_bool quit_;

    const pid_t threadId_; //用来记录当前的eventloop所在线程的id

    //poller返回发生事件的channnel的时间点
    Timestamp pollReturnTime_; 

    std::unique_ptr<Poller> poller_;
    
    /*
    主要作用，当mainLoop获取一个新用户的channel，
    通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
    值得注意的是wakeupFd_和其对应的channel是属于subloop的,subloop对应的
    poller 会监听wakeupfd_
    会有一个wakeupchannel_来标识wakeupFd_
    */
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    /*
    标识当前的loop是否需要执行回调操作
    */
    std::atomic_bool callingPendingFunctors_;

    /*
    存储当前loop需要执行的所有的回调操作
    */
    std::vector<Functor> pendingFunctors_;

    /*
    互斥锁， 保护上面的pendingFunctors_的线程安全
    */
    std::mutex mutex_;

};