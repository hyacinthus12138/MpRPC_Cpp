#include"EventLoopThreadPoll.h"
#include"EventLoopThread.h"

EventLoopThreadPoll::EventLoopThreadPoll(EventLoop *baseLoop, const std::string& nameArg)
    : baseLoop_(baseLoop)
    , name_(nameArg)
    , started_(false)
    , numThreads_(0)
    , next_(0)
{};
EventLoopThreadPoll::~EventLoopThreadPoll(){};

void EventLoopThreadPoll::start( const ThreadInitCallback &cb){
    started_ = true;
    //  std::cout << "in file eventloopthreadpoll.cpp numThreads_ = " 
            // << numThreads_ << std::endl;
    for(int i = 0; i < numThreads_; ++i){
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        // if(t != NULL){
        //     std::cout << "in file eventloopthreadpoll.cpp success cread thread  " 
        //     << i << std::endl;
        // }
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));

        //startLoop会创建一个新线程， 并且和一个subloop绑定（one loop per thread)
        //startLoop 会返回 新的subloop的地址
        loops_.push_back(t->startloop());

        // std::cout << "successfullly " << i << std::endl;

        // 整个服务端只有一个线程，运行着baseloop
        if (numThreads_ == 0 && cb)
        {
            cb(baseLoop_);
        }
    }
}


// 如果工作在多线程中，baseLoop_默认以轮询的方式分配channel给subloop
EventLoop* EventLoopThreadPoll::getNextLoop(){
    EventLoop *loop = baseLoop_;

    if (!loops_.empty()) // 通过轮询获取下一个处理事件的loop
    {
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size())
        {
            next_ = 0;
        }
    }

    return loop;
};
std::vector<EventLoop*> EventLoopThreadPoll::getAllLoops(){
    if (loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
    return {};
};