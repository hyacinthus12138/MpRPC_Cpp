#pragma once

#include"noncopyable.h"
#include<memory>
#include<thread>
#include<string>
#include<atomic>
#include<unistd.h>
#include<functional>


class  Thread : noncopyable{

public:
    using ThreadFunc = std::function<void()>;

    //这里构造函数用explicit修饰， 防止发生隐形转换
    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();

    void start(); //开始执行线程， 注意， 调用start的时候 线程才真正开始执行
    void join();

    bool started() const{ return started_; };
    pid_t tid() const{ return tid_; };
    const std::string &name() const { return name_; };
    static int numCreated() { return numCreated_; };

private:
    //线程如果没有指定名字需要有一个默认名字， 另外 numcreated_ 会在这个函数
    //进行修改， 另外，这个函数在构造函数中一定会被调用
    void setDefaultName();

    bool started_; //标识进程是否开始执行
    bool joined_; //标识进程是否被join回收


    /*
        这里要用一个智能指针来修饰， 不能直接创建 std:thread thread_;
        因为这样线程就会直接执行， 我们这里只是使用一个指针声明一个线程
    */
    std::shared_ptr<std::thread> thread_;

    pid_t tid_;
    ThreadFunc func_; //线程函数
    std::string name_; //线程的名字

    //总共创建了多少线程， 原子操作-》线程安全
    //另外要在类外赋值
    static  std::atomic_int numCreated_;
};