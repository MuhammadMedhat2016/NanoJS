#include "FunctionCreator.hpp"

FunctionCreator::FunctionCreator(v8::Isolate *isolate, const char *functionName, void (*function)(const v8::FunctionCallbackInfo<v8::Value> &args))
    : isolate(isolate), name(StaticHelpers::ToLocalString(isolate, functionName)), functionInstance(v8::FunctionTemplate::New(isolate, function))
{}

bool FunctionCreator::attachMethodToObject(v8::Local<v8::Object> object)
{
    auto context = object->CreationContext();
    object->Set(context, name, this->createInstance(context));
}
v8::Local<v8::Function> FunctionCreator::createInstance(v8::Local<v8::Context> context)
{
    return functionInstance->GetFunction(context).ToLocalChecked();
}
