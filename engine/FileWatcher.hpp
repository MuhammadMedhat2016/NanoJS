#pragma once
#include "EventLoop.hpp"
#include <thread>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
class FileWatcher
{
public:
    static EventLoop* loop;
    static void watch(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void watch(const char *path, callbackJob *job, u_int32_t pollTimeInterval);

private:
};