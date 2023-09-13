#pragma once

#include "v8.h"
#include "EventLoop.hpp"
#include "StaticHelpers.hpp"
#include <memory>
class Buffer
{
private:
    static void writeUtf8(v8::Local<v8::Uint8Array> array, v8::Local<v8::String> str, int64_t offset, int64_t length);
public:
    static EventLoop *loop;
    static 	Local<ObjectTemplate> global;
    static v8::Local<v8::Context> ctx;
    static void fill(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void byteLengthUtf8(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void utf8Write(const v8::FunctionCallbackInfo<v8::Value> &args);
    static int utf8Write(v8::Local<v8::Uint8Array>, v8::Local<v8::String>, int64_t offset, int64_t length);
    static void ucs2Write(const v8::FunctionCallbackInfo<v8::Value> &args);
    static int ucs2Write(v8::Local<v8::Uint8Array> array, v8::Local<v8::String> str, int64_t offset, int64_t length);
    static void fromUtf8(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void fromUtf16(const v8::FunctionCallbackInfo<v8::Value> &args);
};