#pragma once
#include<iostream>
#include<string>
using namespace std;
class TimeStamp{
public:
    TimeStamp();
    explicit TimeStamp(int64_t mircoSeconds);
    string toString() const;
    static TimeStamp now();
private:
    int64_t _mircoSeconds;
};