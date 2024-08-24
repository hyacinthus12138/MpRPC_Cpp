#include<time.h>
#include<iostream>
#include"Timestamp.h"


Timestamp::Timestamp():microSecondsSinceEpoch_(0){};

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    :microSecondsSinceEpoch_(microSecondsSinceEpoch)
    {};


//获取当前时间
Timestamp Timestamp::now(){ 
    return Timestamp(time(NULL));
    //这里其实是调用了Timestamp的有参构造函数， 
    //now函数是static定义的，相当于把当前的时间赋值给
    //microSecondsSinceEpoch_

}; 

std::string Timestamp::toString() const{
    char buf[128] = {0};
    tm *tm_time  = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
        tm_time->tm_year + 1900,
        tm_time->tm_mon + 1,
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec);
    return buf;

};
