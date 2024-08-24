#pragma once

#include<functional>
#include<string>
#include<vector>
#include<memory>
#include<iostream>

#include"noncopyable.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPoll : noncopyable{

public:

    using ThreadInitCallback = std::function<void(EventLoop*)>;

    /*
    构造函数的baseLoop 就是模型的io线程， 也就是 mainLoop， 他会绑定acceptor
    这个mainLoop 由用户创建， nameArg 就是对应的名字
    */
    EventLoopThreadPoll(EventLoop *baseLoop, const std::string& nameArg);
    ~EventLoopThreadPoll();

    void setThreadNum(int numThreads){
       
        numThreads_ = numThreads;
        //  std::cout << "in file eventloopthreadpoll.h numThreads_ = " 
            // << numThreads_ << std::endl;
    }

    void start( const ThreadInitCallback &cb = ThreadInitCallback());

    // 如果工作在多线程中，baseLoop_默认以轮询的方式分配channel给subloop
    EventLoop* getNextLoop();
    std::vector<EventLoop*> getAllLoops();

    bool started() const {return started_;};
    const std::string name() const {return name_;};

private:

    EventLoop *baseLoop_; //这个baseLoop_在构造函数中由用户显示指定
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;

    //里面存储的是指向各个subloop的指针
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    //one loop per thread, 线程和事件循环是一一对应的
    std::vector<EventLoop*> loops_;

};