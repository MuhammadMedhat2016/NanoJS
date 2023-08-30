#include "FileSystem.hpp"

EventLoop *File::loop;
static std::mutex mutex;

void readAsync(const char *path, callbackJob *job, void (*RegisterCallback)(callbackJob *j))
{
    std::ifstream myfile(path, std::ios::in);
    char *buffer = new char[1024];
    myfile.read(buffer, 1024);
    job->args->push_back(StaticHelpers::ToLocalString(File::loop->isolate, buffer));
    RegisterCallback(job);
    myfile.close();
}

void writeAsync(const char *path, const char *data, callbackJob *job, void (*callback)(callbackJob *j))
{
    std::ofstream myfile(path);
    myfile << data;
    char *bytesWritten = new char(20);
    bytesWritten = std::strcpy(bytesWritten, std::to_string(std::strlen(data)).c_str());
    job->args->push_back(StaticHelpers::ToLocalString(File::loop->isolate, bytesWritten));
    callback(job);
    myfile.close();
}
bool checkFileExistence(v8::Isolate *isolate, const char *path)
{
    std::ifstream myfile(path, std::ios::in);
    if (myfile.is_open())
    {
        return true;
        myfile.close();
    }
    return false;
}
void statsAsync(const char *path, callbackJob *job, void (*callback)(callbackJob *j))
{
    struct stat *buffer = nullptr;
    int ret = stat(path, buffer);
    printf("%s \n", path);
    // ObjectCreator stats(v8::Isolate::GetCurrent(), "stats");
    if (ret == -1)
    {
        printf("invalid");
    }
    // stats.SetPropertyValue("mode", buffer->st_mode)

    // delete buffer;
}
std::string File::readFileSync(const char *path)
{

    std::ifstream file(path, std::ios::in);
    if (file.is_open())
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        return buffer.str();
    }
    else
    {
        return "Unable to read the file maybe the file does not exist or the path is wrong";
    }
}
void File::readFileAsync(const char *path, callbackJob *j, void (*callback)(callbackJob *j))
{
    std::thread thread(readAsync, path, j, callback);
    thread.detach();
}
void registerCallback(callbackJob *job)
{
    std::lock_guard<std::mutex> lock(mutex);
    File::loop->addCallbackJob(job);
}
void File::readFileSync(const FunctionCallbackInfo<Value> &args)
{
    v8::Local<Value> v8Path = args[0];
    const char *path = StaticHelpers::ToString(args.GetIsolate(), v8Path);
    std::string res = File::readFileSync(path);
    v8::Local<v8::String> result = StaticHelpers::ToLocalString(args.GetIsolate(), res.c_str());
    args.GetReturnValue().Set(result);
}

void File::readFileAsync(const FunctionCallbackInfo<Value> &args)
{

    v8::Local<Value> v8Path = args[0];
    v8::Local<v8::Function> callback = args[1].As<v8::Function>();
    const char *path = StaticHelpers::ToString(args.GetIsolate(), v8Path);
    callbackJob *job = new callbackJob();
    job->func.Reset(args.GetIsolate(), callback);
    job->args = new std::vector<v8::Local<v8::Value>>();
    File::loop->registerJob();
    File::readFileAsync(path, job, registerCallback);
}

void File::writeFileSync(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();

    v8::Local<v8::String> v8Path = args[0].As<v8::String>();
    v8::Local<v8::String> v8Data = args[1].As<v8::String>();

    const char *path = StaticHelpers::ToString(isolate, v8Path);
    const char *data = StaticHelpers::ToString(isolate, v8Data);

    std::ofstream myfile(path, std::ios::out);

    myfile << data;
}

void File::writeFileAsync(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();

    v8::Local<v8::String> v8Path = args[0].As<v8::String>();
    v8::Local<v8::String> v8Data = args[1].As<v8::String>();
    v8::Local<v8::Function> callback = args[2].As<v8::Function>();
    const char *path = StaticHelpers::ToString(isolate, v8Path);
    const char *data = StaticHelpers::ToString(isolate, v8Data);

    callbackJob *job = new callbackJob();

    job->func.Reset(isolate, callback);
    job->args = new std::vector<v8::Local<Value>>();
    File::loop->registerJob();
    File::writeFileAsync(path, data, job, registerCallback);
}
void File::writeFileAsync(const char *path, const char *data, callbackJob *job, void (*callback)(callbackJob *j))
{
    std::thread thread(writeAsync, path, data, job, callback);
    thread.detach();
}

void File::getStatsAsync(const v8::FunctionCallbackInfo<v8::Value> &args)
{

    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::Local<v8::String> v8Path = args[0].As<v8::String>();
    v8::Local<v8::Function> callback = args[1].As<v8::Function>();
    const char *path = StaticHelpers::ToString(isolate, v8Path);
    if (!checkFileExistence(isolate, path))
    {
        v8::Local<v8::String> error_message = StaticHelpers::ToLocalString(isolate, "file not found!");
        isolate->ThrowException(error_message);
        return;
    }
    callbackJob *job = new callbackJob();
    job->func.Reset(isolate, callback);
    job->args = new std::vector<v8::Local<v8::Value>>();
    // File::loop->registerJob();
    File::getStatsAsync(path, job, registerCallback);
}
void File::getStatsAsync(const char *path, callbackJob *job, void (*callback)(callbackJob *j))
{
    std::thread thread(statsAsync, path, job, callback);
    thread.detach();
}
