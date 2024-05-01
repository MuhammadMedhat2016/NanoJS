#pragma once
#include "EventLoop.hpp"
#include "v8.h"
#include <chrono>
#include <vector>
#include <mutex>

class Timers
{
private:
public:
    static EventLoop* loop;
    static void setTimeOut(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void setInterval(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void registerTimer(TimedJob* job);
};