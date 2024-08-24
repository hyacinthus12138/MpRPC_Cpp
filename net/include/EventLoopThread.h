#pragma once

#include"noncopyable.h"

#include<functional>
#include<string>
#include"Thread.h"
#include<mutex>
#include<condition_variable>

class EventLoop;

class EventLoopThread : noncopyable{

public:

    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
        const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop *startloop();

private:
    void threadFunc(); //线程要执行的线程函数

    EventLoop* loop_; //事件循环
    bool exiting_;
    Thread thread_; //运行事件循环的线程
    
    std::mutex mutex_;
    std::condition_variable con_;
    ThreadInitCallback callback_;

};