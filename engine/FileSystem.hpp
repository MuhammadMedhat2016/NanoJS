#pragma once
#include <fstream>
#include <sstream>
#include <functional>
#include <thread>
#include "Environment.hpp"
#include <queue>
#include <cstring>
#include "StaticHelpers.hpp"
#include "EventLoop.hpp"
#include <mutex>
#include <sys/stat.h>
#include "ObjectCreator.hpp"

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
    static void getStatsAsync(const char*, callbackJob*, void (*callback)(callbackJob *));
    static std::string readFileSync(const char *);
    static void readFileAsync(const char *, callbackJob *, void (*callback)(callbackJob *));
    static void writeFileAsync(const char *, const char *, callbackJob *, void (*callback)(callbackJob *));
};