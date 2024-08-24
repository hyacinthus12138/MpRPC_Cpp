/*
accept 包含 fd， 封装fd的channel， acceptor运行在mainloop，也就是
用户定义的 eventloop
*/

#pragma once

#include"noncopyable.h"
#include"Socket.h"
#include"Channel.h"
#include"InetAddress.h"

#include<functional>


class EventLoop;

class Acceptor : noncopyable{
public:
    /*
        程序启动后， 当有用户请求连接， acceptor 需要对coonfd 进行处理，
        也就是执行对应的callback（ 需要把coonfd封装为channel， 唤醒一个
        subloop， 把coonfd插入subloop）
        具体是怎么实现呢， acceptfd 对应的 acceptchannel 需要注册 读事件
        相应的cb， 这个cb就是 acceptor 的 handleRead函数， 这样当mainloop监听的
        acceptfd 有读事件到达的时候， acceptchannel 会调用注册的handleRead函数
        handleRead函数会调用 NewConnectionCallback, 他的实现由 tcpserver指定，
        NewConnectionCallback 负责唤醒 subloop
    */
    using NewConnectionCallback = std::function<void(int sockfd,
                                    const InetAddress&)>;


    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb){
        newConnectionCallback_ = std::move(cb);
    };

    bool listening() const {return listening_; };
    void listen();

private:

    void handleRead();

    /*
    acceptor本身并不是一个线程， 他运行在baseLoop | mainLoop | io线程中
    初始化时由用户指定， loop_指定acceptor属于的mainLoop
    */
    EventLoop *loop_;
    Socket acceptSocket_;//acceptor对应的fd， 用户连接acceptsocket
    Channel acceptChannel_; //封装acceptor对应的fd

    NewConnectionCallback newConnectionCallback_;
    bool listening_;




    
};