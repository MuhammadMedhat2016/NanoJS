#pragma once
#include <fstream>
#include <sstream>
#include <functional>
#include <queue>
#include <cstring>
#include <mutex>
#include <sys/stat.h>
#include "ObjectCreator.hpp"
#include "EventLoop.hpp"
#include "Buffer.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#include <future>
#include <memory>
#include "ObjectCreator.hpp"
#include <exception>

class File
{

private:
public:
    static EventLoop *loop;
    static void readFileSync(const v8::FunctionCallbackInfo<v8::Value> &);
    static void readFileAsync(const v8::FunctionCallbackInfo<v8::Value> &);
    static void writeFileSync(const v8::FunctionCallbackInfo<v8::Value> &);
    static void writeFileAsync(const v8::FunctionCallbackInfo<v8::Value> &);
    static void getStatsAsync(const v8::FunctionCallbackInfo<v8::Value> &);
    static void getStatsSync(const v8::FunctionCallbackInfo<v8::Value> &);
    static void watchFile(const v8::FunctionCallbackInfo<v8::Value> &args);

    static std::string readFileSync(const char *);
};