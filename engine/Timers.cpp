#include "Timers.hpp"

static std::mutex mutexx;
EventLoop *Timers::loop = nullptr;

void Timers::setTimeOut(const v8::FunctionCallbackInfo<v8::Value> &args)
{

    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();

    v8::Local<v8::Function> callback = args[0].As<v8::Function>();
    time_t duration = args[1].As<v8::Integer>()->Value();

    TimedJob *tJob = new TimedJob();
    tJob->startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tJob->func.Reset(isolate, callback);
    tJob->args = new std::vector<v8::Persistent<v8::Value>>(args.Length() - 2);
    tJob->context.Reset(isolate, context);
    tJob->isInterval = false;
    tJob->duration = duration;
    for (int i = 2; i < args.Length(); ++i)
        (*tJob->args)[i - 2].Reset(isolate, args[i]);

    Timers::loop->updateTime();
    tJob->timeOut = Timers::loop->getLoopTime() + duration;
    Timers::loop->registerJob();

    Timers::loop->addJobToTimersHeap(tJob);
}

void Timers::setInterval(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();

    v8::Local<v8::Function> callback = args[0].As<v8::Function>();
    time_t duration = args[1].As<v8::Integer>()->Value();

    TimedJob *tJob = new TimedJob();
    tJob->startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tJob->func.Reset(isolate, callback);
    tJob->args = new std::vector<v8::Persistent<v8::Value>>(args.Length() - 2);
    tJob->context.Reset(isolate, context);
    tJob->isInterval = true;
    tJob->duration = duration;
    for (int i = 2; i < args.Length(); ++i)
        (*tJob->args)[i - 2].Reset(isolate, args[i]);

    Timers::loop->updateTime();
    tJob->timeOut = Timers::loop->getLoopTime() + duration;
    Timers::loop->registerJob();

    Timers::loop->addJobToTimersHeap(tJob);
}
