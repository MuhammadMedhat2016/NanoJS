#pragma once
#include <queue>
#include "Environment.hpp"
#include "StaticHelpers.hpp"
#include <vector>
#include <chrono>
#include <typeinfo>
#include <string.h>
#include <mutex>
#include <future>
#include <thread>
struct Job
{
    v8::Persistent<v8::Function> *func;
    v8::Persistent<v8::Context> *context;
    std::vector<v8::Persistent<v8::Value>> *args;

    int argc;
    virtual void dummy(){};
};
struct callbackJob : public Job
{
    std::vector<v8::Persistent<v8::Value>> *additionalData;
};

struct TimedJob : public Job
{
    time_t startTime;
    time_t timeOut;
    time_t duration;
    bool isInterval;
};
class TimedJobComparator
{
public:
    bool operator()(const TimedJob *job1, const TimedJob *job2)
    {
        if (job1->timeOut > job2->timeOut)
            return true;
        else if (job1->timeOut == job2->timeOut)
            return job1->startTime < job2->startTime;
        return false;
    }
};
// extern std::vector<std::thread> thread_pool;

class EventLoop
{
private:
    std::queue<callbackJob *> *callbackQueue;
    std::queue<TimedJob *> *timersQueue;
    int jobCount;
    bool shouldStop;
    time_t startTime;
    time_t currentTime;
    std::priority_queue<TimedJob *, std::vector<TimedJob *>, TimedJobComparator> heap;
    void runTimers();
    void runCallbacks();
    void updateTimers();

public:
    v8::Isolate *isolate;
    void runJob(Job *job);
    EventLoop(v8::Isolate *);
    void addCallbackJob(callbackJob *job);
    void addTimedJob(TimedJob *job);
    void removeJob(Job *);
    void registerJob();
    void addJobToTimersHeap(TimedJob *job);
    void removeJobFromTimersHeap();

    TimedJob *getTopTimer();
    time_t getLoopTime();
    void updateTime();
    void init();
    void Run();
    ~EventLoop();
};