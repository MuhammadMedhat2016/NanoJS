#include "EventLoop.hpp"

EventLoop::EventLoop(v8::Isolate *isolate)
    : jobCount(0), shouldStop(true)
{
    this->callbackQueue = new std::queue<callbackJob *>();
    this->timersQueue = new std::queue<TimedJob *>();
    this->isolate = isolate;
    this->startTime = this->currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

void EventLoop::addCallbackJob(callbackJob *job)
{
    this->callbackQueue->push(job);
}
void EventLoop::addTimedJob(TimedJob *job)
{
    this->timersQueue->push(job);
}
void EventLoop::registerJob()
{
    this->jobCount++;
    this->shouldStop = false;
}
void EventLoop::removeJob(Job *job)
{
    callbackJob* ptr = dynamic_cast<callbackJob*>(job);
    if(ptr != NULL)
        this->callbackQueue->pop();
    else 
        this->timersQueue->pop();
    job->func.Reset();
    delete job->args;
    delete job;
    this->jobCount--;
    if (this->jobCount == 0)
        this->shouldStop = true;
}

void EventLoop::runJob(Job *job)
{
    job->argc = job->args->size();
    v8::Local<v8::Value> args[job->argc];
    for (int i = 0; i < job->args->size(); ++i)
        args[i] = (*job->args)[i];
    v8::Local<v8::Function> function = job->func.Get(isolate);
    function->Call(job->context.Get(isolate), v8::Undefined(isolate), job->argc, args);
    this->removeJob(job);
}
void EventLoop::Run()
{
    while (!this->shouldStop)
    {
        this->runTimers();
        this->runCallbacks();
        this->updateTimers();
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
        while (head->timeOut <= this->getLoopTime())
        {
            this->addTimedJob(head);
            this->removeJobFromTimersHeap();
            head = this->getTopTimer();
        }
    }
}
time_t EventLoop::getLoopTime()
{
    return this->currentTime - this->startTime;
}

void EventLoop::updateTime()
{
    this->currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
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
