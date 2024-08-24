#pragma once

//目的是为了使得派生类无法进行拷贝构造和赋值， 但是可以正常的进行构造和析构



class noncopyable{
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

