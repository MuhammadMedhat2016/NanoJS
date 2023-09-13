#pragma once
#include <fstream>
#include <sstream>
#include <functional>
#include <thread>
#include <queue>
#include <cstring>
#include <mutex>
#include <sys/stat.h>
#include <sys/types.h>
#include "ObjectCreator.hpp"
#include "EventLoop.hpp"
#include "Buffer.hpp"
#include <sys/types.h>
#include <sys/socket.h>

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
    static void getStatsAsync(const char *, callbackJob *, void (*callback)(callbackJob *));
    static void getStatsSync(const v8::FunctionCallbackInfo<v8::Value> &);
    static std::string readFileSync(const char *);
    static void readFileAsync(const char *, callbackJob *, v8::Local<v8::Object> options);
    static void writeFileAsync(const char *, const char *, callbackJob *, void (*callback)(callbackJob *));
};