#include"EPollPoller.h"
#include"Logger.h"
#include"Channel.h"


#include<errno.h>
#include<unistd.h>
#include<strings.h>

/*
值得注意的是这三个变量是用来对Channel进行修饰的
kNew -> channel未添加到poller中
kAdded -> channel已添加到poller中
kDeleted -> channel从poller中删除
*/
const int kNew = -1; 
const int kAdded = 1;
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop)
    :Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize){
        if(epollfd_ < 0){
            LOG_FATAL("epoll create error -> fatal : %d\n", errno);
        }
    }
EPollPoller::~EPollPoller(){
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
    //这里其实用 logdebug
    LOG_INFO("func = %s -> fd total count:%d\n",
             __FUNCTION__, static_cast<int>(channels_.size()));
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), 
                    static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0){
        LOG_INFO("%d events happened \n", numEvents);

        //把接收到的读写事件通过 activeChannels 返回给 eventloop
        fillActiveChannels(numEvents, activeChannels);

        if(numEvents == events_.size()){
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0){
        LOG_DEBUG("%s timeout \n", __FUNCTION__);
    }
    else{
        //EINTR 表示系统调用被中断
        if(saveErrno != EINTR){
            errno = saveErrno;
            LOG_ERROR("epollPOller::poll error");
        }
    }
    return now;
};

/*
填写活跃连接通知eventloop
eventlist->events_ 通过epoll wait 获得读写事件， 
再通过activechannel通知eventloop
*/
void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const{
    for(int i = 0; i < numEvents; ++i){
        Channel * channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);

    }
}

/*
    channel发生变化后， eventloop， poller， channel都要发生变化
    具体到poller， 主要是 channelmap 和 epoll对象
    值得注意的是 updatechannel 和 removeChannel 是由eventloop调用的

    channel update remove => EventLoop updateChannel removeChannel => Poller updateChannel removeChannel
 
 *            EventLoop  =>   poller.poll
 *     ChannelList      Poller
 *                     ChannelMap  <fd, channel*>   epollfd

*/
void EPollPoller::updateChannel(Channel *channel){

    const int index = channel->index(); //获得channel的状态
    LOG_INFO(
        "func = %s -> fd = %d events = %d index = %d \n",
        __FUNCTION__, channel->fd(), channel->events(), index
    );
    if(index == kNew || index == kDeleted){
        if(index == kNew){
            int fd = channel->fd();
            channels_[fd] = channel; //ChannelMap::channels_
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else{
        //代表之前这个channel已经在epoll注册过了
        int fd = channel->fd();
        if(channel->isNoneEvent()){
            //如果channel不再关注事件， 就应该从epoll中删除
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else{
            //如果还是关注读写事件， 那么就是要修改读写事件
            update(EPOLL_CTL_MOD, channel);
        }
    }


};
void EPollPoller::removeChannel(Channel *channel){

    int fd = channel->fd();
    channels_.erase(fd);
    LOG_INFO("fun = %s -> fd = %d\n", __FUNCTION__, fd);
    int index = channel->index();
    if(index == kAdded){
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
};

void EPollPoller::update(int operation, Channel *channel){

    //channel要先修改自己对应的event， poller再据此进行修改 

    epoll_event event;
    bzero(&event, sizeof(event));

    int fd = channel->fd();
    event.events = channel->events();

    //这里为什么要注册event.data可以看fillactiveChannels函数
    event.data.fd = fd;
    event.data.ptr = channel;

    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0){
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
};