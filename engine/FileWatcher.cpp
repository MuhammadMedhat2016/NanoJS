#include "FileWatcher.hpp"


EventLoop *FileWatcher::loop;
void cpyJob(callbackJob *srcJob, callbackJob *destJob)
{
    auto isolate = FileWatcher::loop->isolate;
    destJob->args = new std::vector<v8::Persistent<v8::Value>>(0);
    destJob->context->Reset(isolate, srcJob->context->Get(isolate));
    destJob->func->Reset(isolate, srcJob->func->Get(isolate));
}
bool areEqual(char *str1, char *str2, int len)
{
    for (int i = 0; i < len; ++i)
    {
        if (str1[i] != str2[i])
            return false;
    }
    return true;
}

void FileWatcher::watch(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    v8::Local<v8::String> path = args[0].As<v8::String>();
    v8::Local<v8::Object> options = args[1].As<v8::Object>();
    v8::Local<v8::Function> callback = args[2].As<v8::Function>();

    callbackJob *job = new callbackJob();
    job->func = new v8::Persistent<v8::Function>();
    job->context = new v8::Persistent<v8::Context>();
    
    job->context->Reset(isolate, isolate->GetCurrentContext());
    job->func->Reset(isolate, callback);
    job->args = new std::vector<v8::Persistent<v8::Value>>(0);
    FileWatcher::loop->registerJob();
    v8::Local<v8::Value> interval = options->Get(isolate->GetCurrentContext(), StaticHelpers::ToLocalString(isolate, "interval")).ToLocalChecked();
    FileWatcher::watch(StaticHelpers::ToUtf8String(isolate, path), job, interval.As<v8::Integer>()->Value());
}
void watcher(const char *path, callbackJob *job, u_int32_t pollTimeInterval)
{
    auto isolate = FileWatcher::loop->isolate;
    std::ifstream in1(path);
    struct stat buffer;
    int ret = stat(path, &buffer);
    int prevSz = buffer.st_size;
    char *prevContent = new char[prevSz];
    in1.read(prevContent, prevSz);
    in1.close();

    while (true)
    {
        std::ifstream in2(path);
        struct stat curBuffer;
        ret = stat(path, &curBuffer);
        int sz = curBuffer.st_size;
        char *content = new char[sz];
        in2.read(content, sz);
        in2.close();
        if ((sz == prevSz && !areEqual(prevContent, content, sz)) || sz != prevSz)
        {
            delete[] prevContent;
            prevContent = content;
            prevSz = sz;
            printf("%s\n", "change watched");
            
            callbackJob *localJob = new callbackJob();
            cpyJob(job, localJob);
            FileWatcher::loop->registerJob();
            FileWatcher::loop->addCallbackJob(localJob);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(pollTimeInterval));
    }

}

void FileWatcher::watch(const char *path, callbackJob *job, u_int32_t pollTimeInterval)
{
    std::thread thread(watcher, path, job, pollTimeInterval);
    thread.detach();
}
