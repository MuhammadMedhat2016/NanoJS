#pragma once
#include <queue>
#include "Environment.hpp"
#include "StaticHelpers.hpp"
#include <vector>
#include <chrono>
#include <typeinfo>
#include <string.h>
#include <mutex>
#include <thread>
#include <any>
#include <typeinfo>
#include <stdexcept>
#include <variant>
#include <type_traits>
#include <utility>

class value
{
public:
    template <typename T>
    void setValue(const T &value)
    {
        storedValue_ = std::make_any<T>(value);
    }

    template <typename T>
    T getValue() const
    {
        try
        {
            return std::any_cast<T>(storedValue_);
        }
        catch (const std::bad_any_cast& e)
        {
            printf("Error while converting !!\n");
        }
    }

private:
    std::any storedValue_;
};

struct callbackJob;
struct TimedJob;

struct objectProject
{
    std::vector<std::pair<const char *, std::string>> property;
    std::vector<value*> values;
    ~objectProject();
};

class message
{
public:
    message();
    callbackJob *job;
    std::vector<objectProject*> *objects;
    void build();
    ~message();
};

class msgQueue
{
    std::queue<message *> msgQueue;

public:
    void addMsgJob(message *);
    message *getFront();
    bool empty();
};
struct Job
{
    v8::Persistent<v8::Function> *func;
    v8::Persistent<v8::Context> *context;
    std::vector<v8::Persistent<v8::Value>> *args;
    std::vector<v8::Persistent<v8::Value>> *additionalData;
    int argc;
    bool on = true;
    virtual void dummy(){};
    ~Job();
};
struct callbackJob : public Job
{
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

class EventLoop
{
private:
    int jobCount;
    bool shouldStop;
    time_t startTime;
    time_t currentTime;
    std::priority_queue<TimedJob *, std::vector<TimedJob *>, TimedJobComparator> heap;
    void runTimers();
    void runCallbacks();
    void updateTimers();

public:
    std::queue<callbackJob *> *callbackQueue;
    std::queue<TimedJob *> *timersQueue;
    msgQueue *mQu;
    v8::Isolate *isolate;
    void runJob(Job *job);
    EventLoop(v8::Isolate *);
    void addCallbackJob(callbackJob *job);
    void addTimedJob(TimedJob *job);
    void removeJob(Job *);
    void registerJob();
    void addJobToTimersHeap(TimedJob *job);
    void removeJobFromTimersHeap();
    void withdrawJob();

    TimedJob *getTopTimer();
    time_t getLoopTime();
    void updateTime();
    void init();
    void Run();
    ~EventLoop();
};
