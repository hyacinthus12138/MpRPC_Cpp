#include"Thread.h"
#include"CurrentThread.h"
#include<semaphore.h>

std::atomic_int Thread::numCreated_ (0);

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
    , name_(name)
{
    setDefaultName();
}

Thread::~Thread(){
    if(started_ && !joined_){
        thread_ ->detach();// 分离线程
    }
}

//使用start的时候， 新线程才会真正被创建并执行
//而创建的thread对象记录了一个新线程的详细信息
void Thread::start(){
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);

    //真正开启新线程
    thread_ = std::shared_ptr<std::thread>( new std::thread(
        [&](){
            //获取新线程的pid
            tid_ = CurrentThread::tid();

            sem_post(&sem);

            //新线程要执行的线程函数
            func_();
        }
    ));
    /*
        在上面执行创建线程的函数后， 当前线程不会阻塞， 而是会继续执行，
        问题在于多线程任务中， 有可能当前start函数执行结束后， 新线程还未
        开始执行，这就会出错所以， 使用sem，start函数会阻塞在sem_wait, 直到
        新线程中执行sme_post, 这表示新线程开始正常执行， start可以结束
    */
    sem_wait(&sem);
}

void Thread::join(){
    joined_ = true;
    thread_->join();
}
void Thread::setDefaultName(){
    int num = ++numCreated_; //这里顺便更新了 numCreated_;
    if( name_.empty() ){
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}