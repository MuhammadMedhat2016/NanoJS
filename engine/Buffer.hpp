#pragma once

#include "v8.h"
#include "EventLoop.hpp"
#include "StaticHelpers.hpp"
class Buffer
{
private:
public:
    static EventLoop* loop;
    static void byteLengthUtf8(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void utf8Write(const v8::FunctionCallbackInfo<v8::Value> &args);
    static int utf8Write(v8::Local<v8::Uint8Array>, v8::Local<v8::String>,v8::Local<v8::Integer> offset,v8::Local<v8::Integer> length);
    
};