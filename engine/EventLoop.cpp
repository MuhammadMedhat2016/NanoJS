#include "EventLoop.hpp"

static std::mutex callbackMutex;
static std::mutex timerMutex;
extern v8::Isolate *iso;
extern EventLoop *lp;
objectProject::~objectProject()
{
    for (int i = 0; i < this->values.size(); ++i)
        delete this->values[i];
}
message::message()
{
    objects = new std::vector<objectProject *>();
}
message::~message()
{
    for (int i = 0; i < this->objects->size(); ++i)
        delete (*this->objects)[i];
    delete this->objects;
}
void message::build()
{
    auto isolate = iso;
    v8::Locker locker();
    auto context = job->context->Get(isolate);
    // v8::Context::Scope scope(context);
    job->args = new std::vector<v8::Persistent<v8::Value>>(this->objects->size() + 1);

    (*job->args)[0].Reset(isolate, v8::Null(isolate));

    for (int i = 1; i <= this->objects->size(); ++i)
    {
        v8::Local<v8::Object> newObject = v8::Object::New(isolate);

        for (int j = 0; j < (*this->objects)[i - 1]->property.size(); ++j)
        {

            auto [propertyName, type] = (*this->objects)[i - 1]->property[j];
            if (type == "integer")
            {
                int32_t value = (*this->objects)[i - 1]->values[j]->getValue<int32_t>();
                newObject->Set(context, StaticHelpers::ToLocalString(isolate, propertyName), v8::Integer::New(isolate, value));
            }
            else if (type == "string")
            {
                std::string value = (*this->objects)[i - 1]->values[j]->getValue<std::string>();
                newObject->Set(context, StaticHelpers::ToLocalString(isolate, propertyName), StaticHelpers::ToLocalString(isolate, value.c_str()));
            }
            else if (type == "number")
            {
                double value = (*this->objects)[i - 1]->values[j]->getValue<double>();
                newObject->Set(context, StaticHelpers::ToLocalString(isolate, propertyName), v8::Number::New(isolate, value));
            }
            else if (type == "boolean")
            {
                bool value = (*this->objects)[i - 1]->values[j]->getValue<bool>();
                newObject->Set(context, StaticHelpers::ToLocalString(isolate, propertyName), v8::Boolean::New(isolate, value));
            }
        }

        (*job->args)[i].Reset(isolate, newObject);
    }
    lp->addCallbackJob(this->job);
}

static std::mutex mux;

void msgQueue::addMsgJob(message *msg)
{
    std::lock_guard<std::mutex> guard(mux);
    msgQueue.push(msg);
}
message *msgQueue::getFront()
{
    std::lock_guard<std::mutex> guard(mux);
    message *ptr = msgQueue.front();
    msgQueue.pop();
    return ptr;
}
bool msgQueue::empty()
{
    std::lock_guard<std::mutex> guard(mux);
    return msgQueue.empty();
}
Job::~Job()
{
    this->context->Reset();
    this->func->Reset();
    for (int i = 0; i < this->argc; ++i)
        (*this->args)[i].Reset();
    delete this->additionalData;
    delete this->args;
}
EventLoop::EventLoop(v8::Isolate *isolate)
    : jobCount(0), shouldStop(true)
{
    this->callbackQueue = new std::queue<callbackJob *>();
    this->timersQueue = new std::queue<TimedJob *>();
    this->mQu = new msgQueue();
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
    // v8::Locker locker(isolate);

    // v8::Local<v8::Context> ctx = v8::Context::New(isolate);
    // v8::Context::Scope scope(ctx);

    callbackJob *ptr = dynamic_cast<callbackJob *>(job);

    if (ptr != NULL)
    {
        this->callbackQueue->pop();
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
    delete job;
    this->jobCount--;
    if (this->jobCount == 0)
        this->shouldStop = true;
}

void EventLoop::runJob(Job *job)
{
    auto isolate = iso;
    //    v8::Locker locker(isolate);
    // isolate->Enter();
    auto context = job->context->Get(isolate);
    job->argc = job->args->size();
    v8::Local<v8::Value> args[job->argc];
    for (int i = 0; i < job->argc; ++i)
        args[i] = (*job->args)[i].Get(isolate);
    v8::Local<v8::Function> function = job->func->Get(isolate);
    function->Call(context, v8::Undefined(isolate), job->argc, args);
    this->removeJob(job);
}

void EventLoop::withdrawJob()
{
    this->jobCount--;
    if (this->jobCount == 0)
        this->shouldStop = true;
}

void EventLoop::Run()
{
    while (!this->shouldStop)
    {
        while (!mQu->empty())
        {
            message *msg = mQu->getFront();
            msg->build();
            delete msg;
        }
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
