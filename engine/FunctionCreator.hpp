#pragma once
#include "v8.h"
#include "StaticHelpers.hpp"

class FunctionCreator
{
private:
    v8::Isolate *isolate;
    v8::Local<v8::String> name;
    v8::Local<v8::FunctionTemplate> functionInstance;

public:
    FunctionCreator(v8::Isolate *isolate, const char *functionName, void (*function)(const v8::FunctionCallbackInfo<v8::Value> &args));
    bool attachMethodToObject(v8::Local<v8::Object>);
    v8::Local<v8::Function> createInstance(v8::Local<v8::Context>);
};