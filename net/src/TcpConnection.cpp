#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <functional>
#include <errno.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <string>

static EventLoop* CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d TcpConnection Loop is null! \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop,
                    const std::string &nameArg,
                    int sockfd,
                    const InetAddress &localAddr,
                    const InetAddress &peerAddr)
                : loop_(CheckLoopNotNull(loop))
                , name_(nameArg)
                , state_(kConnecting)
                , reading_(true)
                , socket_(new  Socket(sockfd))
                , channel_(new Channel(loop, sockfd))
                , localAddr_(localAddr)
                , peerAddr_(peerAddr)
                , highWaterMark_(64*1024*1024) //64M
{
    // 下面给channel设置相应的回调函数，
    //poller给channel通知感兴趣的事件发生了，
    //channel会回调相应的操作函数
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1)
    );
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this)
    );
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this)
    );
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this)
    );

    LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d \n", 
        name_.c_str(), channel_->fd(), (int)state_);
}


void TcpConnection::handleRead(Timestamp receiveTime){
    int saveErrno = 0;
    ssize_t  n = inputBuffer_.readFd(channel_->fd(), &saveErrno);

    if(n > 0){
        /*
        成功读取了客户端的数据
        channel 会调用 handleEvent 函数， 最后调用
        构造函数绑定的 TcpConnection::handleRead函数
        也就是这个函数
        TcpConnection::handleRead函数会先使用buffer读取客户端fd上的数据
        ， 然后调用 用户传入的messageCallback_回调操作
        从这个角度看， handleRead 函数是对 messageCallback 的封装
        */
       messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if(n == 0){ //说明对方要关闭连接
        handleClose();
    }
    else{
        errno = saveErrno;
        LOG_ERROR("Tcpconnection::handleRead error");
        handleError();
    }
}

void TcpConnection::handleWrite(){
    /*
        要注意handelWrite 处理的是 发送buffer 里面的数据
        而且只是发送一次， 不一定能发送全部， 不过 handlewrite
        能确保buffer里面的数据被全部发送， 因为只有当数据被全部发送
        之后才会调用channel_->disableWriting();在此之前的话epoll
        会一直通知channel调用 handlewrite

        首先要判断 当前的channel是否还能发送数据
        然后通过 发送buffer 发送数据
        另外 如果所有数据都发送完成了， 就要执行
        用户提前设定的 writeCompleteCallback 回调
    */
    if(channel_->isWriting()){
        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);
        if(n > 0){
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0){
                //没有数据可以发送了
                channel_->disableWriting();
                if(writeCompleteCallback_){
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this())
                    );
                }
                if(state_ == kDisconnecting){
                    shutdownInLoop();
                }
            }
        }
        else{
            LOG_ERROR("Tcpconnection::handleWrite error");
        }
        
    }
    else{
         LOG_ERROR("TcpConnection fd=%d is down, no more writing \n", channel_->fd());
    }
}

// poller => channel::closeCallback => TcpConnection::handleClose
void TcpConnection::handleClose()
{
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d \n", channel_->fd(), (int)state_);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr); // 执行连接关闭的回调
    closeCallback_(connPtr); // 关闭连接的回调  执行的是TcpServer::removeConnection回调方法
}
void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n", name_.c_str(), err);
}

void TcpConnection::send(const std::string &buf){
    //注意这里这里发送的就是 string
    if(state_ == kConnected){ //肯定要在连接状态才可以发数据
        if(loop_->isInLoopThread()){
            //如果是在当前线程， 就直接 sendinLoop 发送
            
            sendInLoop(buf.c_str(), buf.size());
        }
        else{
            //不在当前线程的话， 调用 runinloop 唤醒对应线程执行 sendInloop
            loop_->runInLoop(std::bind(
                &TcpConnection::sendInLoop,
                this,
                buf.c_str(),
                buf.size())
            );
        }
    }
}

/**
 * 发送数据  应用写的快， 而内核发送数据慢，
 *  需要把待发送数据写入缓冲区， 而且设置了水位回调
 */ 
void TcpConnection::sendInLoop(const void* data, size_t len){
    ssize_t nwrote = 0;  //已经发送的数据
    ssize_t remaining = len; //还未发送的数据
    bool faultError = false; //是否有默认错误

    if(state_ == kDisconnected){
        LOG_ERROR("disconnected, give up writing ");
        return;
    }
    /*
    * 一般一个新的connfd到达的时候， 我们只对 read事件 感兴趣
    并不会注册写事件，所以 !channel->isWriting() 表示是第一次开始写数据或者
    说之前的数据已经全部发送完成后导致用户取消了写事件-->disableWriting()
    另外发送缓冲区有数据的话说明也不是第一次发送数据

    所以这里的逻辑就是 发送数据时先调用Linux系统调用write发送数据
        如果全部发送完成就直接结束
        否则，把数据写入发送缓冲区， 注册写事件，poller接收到epollout，
        通知channel 调用 handlewrite 发送剩余数据, 发送完成后取消写事件
    这里就实现了上面说的 发送太慢需要写入buffer的情况
    */
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0){
        nwrote = ::write(channel_->fd(), data, len);
        if(nwrote > 0){
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_){
                // 既然在这里数据全部发送完成，就不用再给channel设置epollout事件了
                //remaining == 0, 后面的程序会直接略过， 函数在这里就结束
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this())
                );
            }
        }
        else {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) // SIGPIPE  RESET
                {
                    faultError = true;
                }
            }
        }
    }

    /*
    没有出错 也 还有剩余数据
    说明当前这一次write，并没有把数据全部发送出去，剩余的数据需要保存到
    缓冲区当中，然后给channel注册epollout事件，注册了epollout事件之后
    poller只要发现tcp的发送缓冲区有空间，就会通知相应的sock-channel，调用writeCallback_回调方法也就是
    调用TcpConnection::handleWrite方法，直到把发送缓冲区中的数据全部发送完成
    这时候 disableWriting---》取消epollout
    */
   if (!faultError && remaining > 0) 
    {
        // 目前发送缓冲区剩余的待发送数据的长度
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_)
        {
            loop_->queueInLoop(
                std::bind(highWaterMarkCallback_, shared_from_this(), oldLen+remaining)
            );
        }
        outputBuffer_.append((char*)data + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting(); // 这里一定要注册channel的写事件，否则poller不会给channel通知epollout
        }
    }
}


void TcpConnection::connectEstablished(){
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading(); // 向poller注册channel的epollin事件

    // 新连接建立，执行回调
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll(); // 把channel的所有感兴趣的事件，从poller中del掉
        connectionCallback_(shared_from_this());
    }  
    channel_->remove(); // 把channel从poller中删除掉
}

// 关闭连接
void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        /*
        注意这里的逻辑， 如果channel 还在执行发送事件的话
        也就是channel-》iswriting == true， 看之前的函数
        这代表 buffer里面有数据要发送

        这样的话这里就只执行 设置 kdisconnecting
        而缓冲区中有数据就会调用handlewrite， 里面的
        if(state_ == kDisconnecting){
                    shutdownInLoop();
        } 
        会再次调用shutdowninloop， 确保关闭tcpconnection
        */
        loop_->runInLoop(
            std::bind(&TcpConnection::shutdownInLoop, this)
        );
    }
}

void TcpConnection::shutdownInLoop()
{
    if (!channel_->isWriting()) // 说明outputBuffer中的数据已经全部发送完成
    {
        socket_->shutdownWrite(); // 关闭写端
    }
}