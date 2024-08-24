#include"EventLoopThread.h"
#include"EventLoop.h"

#include<iostream>

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb,
        const std::string &name)
        : loop_(NULL)
        , exiting_(false)
        , thread_(std::bind(
            &EventLoopThread::threadFunc, this
        ), name) //threadFunc 是线程要执行的线程函数
        , mutex_(), con_(), 
        callback_(cb)//这里函数回调默认是空函数
        {
        }
EventLoopThread::~EventLoopThread(){
    exiting_ = true;
    if(loop_ != NULL){
        loop_->quit();
        thread_.join();
    }

}
EventLoop* EventLoopThread::startloop(){
    /*
    在构造函数中制定了线程的线程函数 threadFunc，
    现在执行thread_.start正式开启新线程
    */
    thread_.start();

    // std::cout << "file eventloopthread" << std::endl;
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while ( loop_ == nullptr )
        {
            con_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}


/*
    这个函数是新线程的线程函数， eventloopthread初始化的时候
    被传入新线程， 他是在新的线程中执行的
*/
void EventLoopThread::threadFunc(){
    /*
        我们在一个新的线程中创建一个新的事件循环-》 one loop per thread
    */
    EventLoop loop;
    if(callback_){
        callback_(&loop);
    }
    {
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = &loop;
    con_.notify_one();
    }

    loop.loop();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

