#pragma once

#include"noncopyable.h"
#include"Timestamp.h"

#include<functional>
#include<memory>


//这里是一个类的前置声明， 因为用到eventloop的时候只使用了
//eventloop的指针

/**
 * 理清楚  EventLoop、Channel、Poller之间的关系   《= Reactor模型上对应 Demultiplex
 * Channel 理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN、EPOLLOUT事件
 * 还绑定了poller返回的具体事件
 */ 

class EventLoop;
class Channel : noncopyable{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    //这是channel类的构造函数， channel表示一个监听的
    //sockfd以及对应的events
    //一个eventloop 对应一个epoll， 管理多个channel， 所以
    //要表示channel哪个eventloop管理
    Channel(EventLoop *loop, int fd);

    ~Channel(){
        //muduo会有多个eventloop， channel必须由管理他的eventloop
        //进行释放， 这里的逻辑忽略。
    }

    //poller通知新的事件， handEvent用来处理对应事件
    //handEvent会进一步调用私有函数 handleEventWithGuard（）
    void handleEvent(Timestamp recerveTime);

    //设置对应的回调函数
    void setReadCallback(ReadEventCallback cb){readCallback_ = cb;};
    void setWriteCallback(EventCallback cb){writeCallback_ = cb;};
    void setCloseCallback(EventCallback cb){closeCallback_ = cb;};
    void setErrorCallback(EventCallback cb){errorCallback_ = cb;};

    // 防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const{return fd_;};
    int events() const {return events_;};
    void set_revents(int revt) {revents_ = revt;} //这个函数是poller调用的， 用来返回到达的事件
     

    //提供修改events的接口
    void enableReading(){events_ |= kReadEvent; update();};
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    // one loop per thread
    EventLoop* ownerLoop() { return loop_; }
    void remove();
private:
    void handleEventWithGuard(Timestamp receiveTime);
    void update(); //epoll_clt

    //这三个变量来表示这个channel的状态
    //值得注意的是静态成员变量要在类外定义， 一般是在cpp文件
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    //channel 会通过 revents 得知发生的具体事件
    //我们要注册回调函数进行处理
    //值得注意的是在这里回调函数并未被具体确定
    //进行实际调用时通过setreadCallback（）函数确定
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

    EventLoop *loop_; //标识该channel由哪一个epoll管理
    const int fd_; //该channel对应的sockfd
    int events_; //该channel对应的sockfd， 我们希望监听哪些事件
    int revents_; //poller会修改revents_,用来返回我们监听到了什么事件
    int index_; //我们使用 index 来记录 channel 与 Poller 相关的几种状态，Poller 类会判断当前 channel 的状态然后处理不同的事情
                //index 和 poller里面的kNew， kAdded， kDeleted一致， 表示该channel在poller中是 没添加
                //被添加， 被删除 这三种状态
    std::weak_ptr<void> tie_;
    bool tied_;
};

