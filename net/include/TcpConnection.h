#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <atomic>

class Channel;
class EventLoop;
class Socket;


/**
 * TcpServer => Acceptor => 有一个新用户连接，通过accept函数拿到connfd
 * =》 TcpConnection 设置回调 =》 Channel =》 Poller =》 Channel的回调操作
 * tcpconnection 和 channel 是一一对应的， connection 是 channel的上一层
 * 这个类主要封装了一个已建立的TCP连接，
 * 以及控制该TCP连接的方法（连接建立和关闭和销毁），
 * 以及该连接发生的各种事件（读/写/错误/连接）对应的处理函数，
 * 以及这个TCP连接的服务端和客户端的套接字地址信息等。
 */

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, 
                const std::string &name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const {return loop_;};
    const std::string& name() const {return name_;};
    const InetAddress& localAddress() const {return localAddr_;};
    const InetAddress& peerAddress() const {return peerAddr_;};
    bool connected() const {return state_ == kConnected;};

    void send(const std::string &buf); //发送数据
    void  shutdown(); //关闭当前的连接

    void setConnectionCallback(const ConnectionCallback &cb){ connectionCallback_ = cb;};
    void setMessageCallback(const MessageCallback &cb){messageCallback_ = cb;};
    void setWriteCompleteCallback(const WriteCompleteCallback& cb){ writeCompleteCallback_ = cb; };
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark){ highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; };
    void setCloseCallback(const CloseCallback& cb){ closeCallback_ = cb; };

    void connectEstablished();
    void connectDestroyed();
private:
    //这个枚举类型对应的是 这条tcpconnection 的当前状态
    std::atomic_int state_;
    enum StateE {kDisconnected, kConnecting, kConnected, kDisconnecting};
    void setState(StateE state) {state_ = state;};

    /*
    这四个函数用来设置 channel 的cb， 也就是 channel当发生duiyingshijian
    的时候， 会调用这四个函数， 这四个函数 实际上时调用了
    private里面的 ConnectionCallback 这四个回调， 而这四个回调是用户
    指定的， 也就是说， 用户 指定 MessageCallback --》 handleRead
    --》 channel-》handleRead
    */
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();

    /*
        mainloop的acceptor把新注册的connfd注册到subloop，
        所以这个loop绝对不是baseLoop， 而是subloop， 所有的tcpconnection
        都是在subloop中

    */
    EventLoop* loop_; 
    const std::string name_; //客户端的名字
    bool reading_; //当前tcpconnection是否正在监听读事件

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    
    const InetAddress localAddr_; //服务器端， 本地ip
    const InetAddress peerAddr_; //客户端ip

    ConnectionCallback connectionCallback_; //有新连接的cb
    MessageCallback messageCallback_; //有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; //消息发送完成的cb
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;

    //接收和发送消息都需要各自缓冲buffer
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};