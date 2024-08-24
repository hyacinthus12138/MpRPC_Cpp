#pragma once
#include<iostream>
#include<string>


class Timestamp{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    static Timestamp now(); //获取当前时间
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;

};
