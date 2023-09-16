#include "EventLoop.hpp"

static std::mutex callbackMutex;
static std::mutex timerMutex;
extern std::vector<std::shared_future<callbackJob *>> vec;

class callbackJob;

EventLoop::EventLoop(v8::Isolate *isolate)
    : jobCount(0), shouldStop(true)
{
    this->callbackQueue = new std::queue<callbackJob *>();
    this->timersQueue = new std::queue<TimedJob *>();
    this->isolate = isolate;
    this->startTime = this->currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) * 1000;
}

void EventLoop::addCallbackJob(callbackJob *job)
{
    std::lock_guard<std::mutex> guard(callbackMutex);
    this->callbackQueue->push(job);
}
void EventLoop::addTimedJob(TimedJob *job)
{

    std::lock_guard<std::mutex> guard(timerMutex);
    this->timersQueue->push(job);
}
void EventLoop::registerJob()
{
    this->jobCount++;
    this->shouldStop = false;
}
void EventLoop::removeJob(Job *job)
{
    callbackJob *ptr = dynamic_cast<callbackJob *>(job);
    if (ptr != NULL)
    {
        this->callbackQueue->pop();
        //callbackJob *Job = reinterpret_cast<callbackJob *>(job);
        //for(int i = 0; i < Job->additionalData->size(); ++i) (*Job->additionalData)[i].Reset();
        //delete Job->additionalData;
    }
    else
    {
        this->timersQueue->pop();
        TimedJob *tJob = reinterpret_cast<TimedJob *>(job);
        if (tJob->isInterval)
        {
            tJob->timeOut = tJob->duration + this->getLoopTime();
            this->addJobToTimersHeap(tJob);
            return;
        }
    }

    job->func->Reset();
    job->context->Reset();
    for (int i = 0; i < job->argc; ++i)
        (*job->args)[i].Reset();
    delete job->args;
    delete job;
    this->jobCount--;
    if (this->jobCount == 0)
        this->shouldStop = true;
}

void EventLoop::runJob(Job *job)
{

    job->argc = job->args->size();
    auto context = job->context->Get(isolate);
    v8::Local<v8::Value> args[job->argc];
    for (int i = 0; i < job->args->size(); ++i)
        args[i] = (*job->args)[i].Get(isolate);
    v8::Local<v8::Function> function = job->func->Get(isolate);
    function->Call(context, v8::Undefined(isolate), job->argc, args);
    this->removeJob(job);
}
void EventLoop::Run()
{
    while (!this->shouldStop)
    {

        this->runTimers();
        this->runCallbacks();
        this->updateTimers();
        for (int i = 0; i < vec.size(); ++i)
        {
            if (vec[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                this->addCallbackJob(vec[i].get());
                vec.erase(vec.begin() + i);
            }
        }
    }
}
void EventLoop::runTimers()
{
    while (!this->timersQueue->empty())
    {
        Job *job = this->timersQueue->front();
        this->runJob(job);
    }
}
void EventLoop::runCallbacks()
{
    while (!this->callbackQueue->empty())
    {
        Job *job = this->callbackQueue->front();
        this->runJob(job);
    }
}
void EventLoop::updateTimers()
{
    this->updateTime();
    if (this->heap.size() != 0)
    {
        TimedJob *head = this->getTopTimer();
        // printf("headTimeout is %d loop time is %d \n", head->timeOut, this->getLoopTime());
        while (this->heap.size() != 0 && head->timeOut <= this->getLoopTime())
        {
            this->addTimedJob(head);
            this->removeJobFromTimersHeap();
            head = this->getTopTimer();
        }
    }
}
time_t EventLoop::getLoopTime()
{
    return this->currentTime;
}

void EventLoop::updateTime()
{
    this->currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) * 1000;
}

void EventLoop::addJobToTimersHeap(TimedJob *job)
{
    this->heap.push(job);
}

TimedJob *EventLoop::getTopTimer()
{
    return this->heap.top();
}

void EventLoop::removeJobFromTimersHeap()
{
    this->heap.pop();
}

EventLoop::~EventLoop()
{
    delete this->callbackQueue;
    delete this->timersQueue;
}
