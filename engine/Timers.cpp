#include "Timers.hpp"

static std::mutex mutexx;
EventLoop *Timers::loop = nullptr;

void Timers::registerTimer(TimedJob *job)
{
    std::lock_guard<std::mutex> guard(mutexx);
    Timers::loop->addTimedJob(job);
}
void Timers::setTimeOut(const v8::FunctionCallbackInfo<v8::Value> &args)
{

    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    
    v8::Local<v8::Function> callback = args[0].As<v8::Function>();
    time_t duration = args[1].As<v8::Integer>()->Value();

    TimedJob *tJob = new TimedJob();
    tJob->startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tJob->func.Reset(isolate, callback);
    tJob->args = new std::vector<v8::Local<v8::Value>>();
    tJob->context.Reset(isolate, context);
    
    tJob->timeOut = Timers::loop->getLoopTime() + duration;
       


    for (int i = 2; i < args.Length(); ++i)
        tJob->args->push_back(args[i]);
     
    Timers::loop->registerJob();
     
    //Timers::loop->addJobToTimersHeap(tJob);
    
}