#include"Channel.h"
#include"EventLoop.h"
#include"Logger.h"

#include"sys/epoll.h"

//static变量一般要在cpp文件定义
// const int Channel::kNoneEvent = 0;
// const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
// const int Channel::kWriteEvent = EPOLLOUT;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)

    :loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    tied_(false)
    {}

// channel的tie方法什么时候调用过？一个TcpConnection新连接创建的时候 TcpConnection => Channel 
void Channel::tie(const std::shared_ptr<void> &obj)

{
    tie_ = obj;
    tied_ = true;
}

//当修改以及删除sockfd所监听的events之后， 我们需要通知poller
//所以要调用poller提供的的接口
void Channel::update(){
    loop_->updateChannel(this);
}
void Channel::remove(){

    loop_->removeChannel(this);
}

//handleEvent函数来处理事件
void Channel::handleEvent(Timestamp receiveTime){

    if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }

}


// 根据poller通知的channel发生的具体事件， 由channel负责调用具体的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d\n", revents_);

    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }

    if (revents_ & EPOLLERR)
    {
        if (errorCallback_)
        {
            errorCallback_();
        }
    }

    if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        if (readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_)
        {
            writeCallback_();
        }
    }
}