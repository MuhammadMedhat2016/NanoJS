#include "FileSystem.hpp"

EventLoop *File::loop;

void registerCallback(callbackJob *job)
{
    File::loop->addCallbackJob(job);
}

extern std::vector<std::shared_future<callbackJob*>> vec;

callbackJob* readAsync(const char *path, callbackJob *job)
{
    //std::this_thread::sleep_for(std::chrono::seconds(5));
    //printf("%d \n",std::this_thread::get_id());
    auto isolate = File::loop->isolate;
    auto context = job->context->Get(isolate);

    std::ifstream myfile(path, std::ios::in);

    int64_t offset, length, position;

    v8::Local<v8::Uint8Array> array;

    (*job->additionalData)[0].Get(isolate)->IntegerValue(context).To(&offset);
    (*job->additionalData)[1].Get(isolate)->IntegerValue(context).To(&length);
    (*job->additionalData)[2].Get(isolate)->IntegerValue(context).To(&position);

    array = (*job->additionalData)[3].Get(isolate).As<v8::Uint8Array>();
    (*job->additionalData)[3].Reset();

    void *ptr = array->Buffer()->GetBackingStore()->Data();
    ptr = (char *)ptr + offset;
    myfile.seekg(position, std::ios::beg);
    myfile.read((char *)ptr, length);

    std::streamsize bytesRead = myfile.gcount();

    myfile.close();

    (*job->args)[0].Reset(isolate, v8::Null(isolate));
    (*job->args)[1].Reset(isolate, array);
    (*job->args)[2].Reset(isolate, v8::Integer::New(isolate, static_cast<int>(bytesRead)));

    return job;
}
void File::readFileAsync(const FunctionCallbackInfo<Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();

    v8::Local<Value> v8Path = args[0].As<v8::String>();
    v8::Local<v8::Object> options = args[1].As<v8::Object>();
    v8::Local<v8::Function> callback = args[2].As<v8::Function>();

    v8::Local<v8::Integer> v8Offset = options->Get(context, v8::String::NewFromUtf8(isolate, "offset").ToLocalChecked()).ToLocalChecked().As<v8::Integer>();
    v8::Local<v8::Integer> v8Length = options->Get(context, v8::String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked().As<v8::Integer>();
    v8::Local<v8::Integer> v8Position = options->Get(context, v8::String::NewFromUtf8(isolate, "position").ToLocalChecked()).ToLocalChecked().As<v8::Integer>();
    v8::Local<v8::Uint8Array> array = options->Get(context, v8::String::NewFromUtf8(isolate, "buffer").ToLocalChecked()).ToLocalChecked().As<v8::Uint8Array>();

    const char *path = StaticHelpers::ToUtf8String(isolate, v8Path);
    callbackJob *job = new callbackJob();

    job->func = new v8::Persistent<v8::Function>();
    job->context = new v8::Persistent<v8::Context>();
    job->args = new std::vector<v8::Persistent<v8::Value>>(3);

    job->func->Reset(isolate, callback);
    job->context->Reset(isolate, context);

    job->additionalData = new std::vector<v8::Persistent<v8::Value>>(4);
    (*job->additionalData)[0].Reset(isolate, v8Offset);
    (*job->additionalData)[1].Reset(isolate, v8Length);
    (*job->additionalData)[2].Reset(isolate, v8Position);
    (*job->additionalData)[3].Reset(isolate, array);

    File::loop->registerJob();
    vec.push_back(std::async(std::launch::async,readAsync, path, job));
    //std::thread thread = std::thread(readAsync, path, job);
    //thread_pool.push_back(thread.);
    //thread.detach();

    //thread.join();
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
}
void File::readFileSync(const FunctionCallbackInfo<Value> &args)
{
    v8::Local<Value> v8Path = args[0];
    const char *path = StaticHelpers::ToUtf8String(args.GetIsolate(), v8Path);
    std::string res = File::readFileSync(path);
    v8::Local<v8::String> result = StaticHelpers::ToLocalString(args.GetIsolate(), res.c_str());
    args.GetReturnValue().Set(result);
}

callbackJob* writeAsync(const char *path, const char *data, callbackJob *job, void (*callback)(callbackJob *j))
{
    //std::this_thread::sleep_for(std::chrono::seconds(3));
    //printf("%d \n",std::this_thread::get_id());
    auto isolate = File::loop->isolate;

    std::ofstream myfile(path);
    myfile << data;
    char *bytesWritten = new char(20);
    myfile.close();

    bytesWritten = std::strcpy(bytesWritten, std::to_string(std::strlen(data)).c_str());
    (*job->args)[0].Reset(isolate, v8::Null(isolate));
    (*job->args)[1].Reset(isolate, StaticHelpers::ToLocalString(isolate, bytesWritten));
    //callback(job);
    return job;
}
void File::writeFileSync(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();

    v8::Local<v8::String> v8Path = args[0].As<v8::String>();
    v8::Local<v8::String> v8Data = args[1].As<v8::String>();

    const char *path = StaticHelpers::ToUtf8String(isolate, v8Path);
    const char *data = StaticHelpers::ToUtf8String(isolate, v8Data);

    std::ofstream myfile(path, std::ios::out);

    myfile << data;
}

void File::writeFileAsync(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = v8::Context::New(isolate);
    v8::Local<v8::String> v8Path = args[0].As<v8::String>();
    v8::Local<v8::String> v8Data = args[1].As<v8::String>();
    v8::Local<v8::Function> callback = args[2].As<v8::Function>();
    const char *path = StaticHelpers::ToUtf8String(isolate, v8Path);
    const char *data = StaticHelpers::ToUtf8String(isolate, v8Data);

    callbackJob *job = new callbackJob();
    job->func = new v8::Persistent<v8::Function>();
    job->context = new v8::Persistent<v8::Context>();
    job->func->Reset(isolate, callback);
    job->context->Reset(isolate, context);
    job->args = new std::vector<v8::Persistent<Value>>(2);

    File::loop->registerJob();
    File::writeFileAsync(path, data, job, registerCallback);
}
void File::writeFileAsync(const char *path, const char *data, callbackJob *job, void (*callback)(callbackJob *j))
{
    auto isolate = File::loop->isolate;
    vec.push_back(std::async(std::launch::async, writeAsync,path, data, job, callback));
    //std::thread thread = std::thread(writeAsync, path, data, job, callback);
    //thread.join();
    //thread_pool.push_back(std::move(thread));
    //thread.detach();
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

void File::getStatsAsync(const v8::FunctionCallbackInfo<v8::Value> &args)
{

    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::Local<v8::String> v8Path = args[0].As<v8::String>();
    v8::Local<v8::Function> callback = args[1].As<v8::Function>();
    const char *path = StaticHelpers::ToUtf8String(isolate, v8Path);

    callbackJob *job = new callbackJob();
    job->func = new v8::Persistent<v8::Function>();
    job->context = new v8::Persistent<v8::Context>();

    job->func->Reset(isolate, callback);
    job->args = new std::vector<v8::Persistent<v8::Value>>(2);
    job->context->Reset(isolate, context);
    File::loop->registerJob();

    File::getStatsAsync(path, job, registerCallback);
}
std::string getPermissions(mode_t mode)
{
    std::string permissions = "";
    if (mode & S_IRUSR)
        permissions = "R";
    else
        permissions = "_";
    if (mode & S_IWUSR)
        permissions += "W";
    else
        permissions += "_";
    if (mode & S_IXUSR)
        permissions += "R";
    else
        permissions += "_";
    if (mode & S_IRGRP)
        permissions += "R";
    else
        permissions += "_";
    if (mode & S_IWGRP)
        permissions += "W";
    else
        permissions += "_";
    if (mode & S_IXGRP)
        permissions += "X";
    else
        permissions += "_";
    if (mode & S_IROTH)
        permissions += "R";
    else
        permissions += "_";
    if (mode & S_IWOTH)
        permissions += "W";
    else
        permissions += "_";
    if (mode & S_IXOTH)
        permissions += "X";
    else
        permissions += "_";
    return permissions;
}
void statsAsync(const char *path, callbackJob *job, void (*callback)(callbackJob *j))
{

    auto isolate = File::loop->isolate;
    auto context = job->context->Get(isolate);
    struct stat buffer;
    int ret = stat(path, &buffer);
    if (ret != 0)
    {
        // TO DO
    }
    else
    {
        (*job->args)[0].Reset(isolate, v8::Undefined(isolate));
        ObjectCreator statsTemp = ObjectCreator(isolate, "statsTemp");
        statsTemp.SetPropertyValue("mode", StaticHelpers::ToLocalString(isolate, getPermissions(buffer.st_mode).c_str()));
        statsTemp.SetPropertyValue("dev", v8::Integer::New(isolate, buffer.st_dev));
        statsTemp.SetPropertyValue("ino", v8::Integer::New(isolate, buffer.st_ino));
        statsTemp.SetPropertyValue("nlink", v8::Integer::New(isolate, buffer.st_nlink));
        statsTemp.SetPropertyValue("uid", v8::Integer::New(isolate, buffer.st_uid));
        statsTemp.SetPropertyValue("gid", v8::Integer::New(isolate, buffer.st_gid));
        statsTemp.SetPropertyValue("rdev", v8::Integer::New(isolate, buffer.st_rdev));
        statsTemp.SetPropertyValue("size", v8::Integer::New(isolate, buffer.st_size));
        statsTemp.SetPropertyValue("blksize", v8::Integer::New(isolate, buffer.st_blksize));
        statsTemp.SetPropertyValue("atimeMs", v8::Integer::New(isolate, buffer.st_atime * 1000));
        statsTemp.SetPropertyValue("mtimeMs", v8::Integer::New(isolate, buffer.st_mtime * 1000));
        statsTemp.SetPropertyValue("ctimeMs", v8::Integer::New(isolate, buffer.st_ctim.tv_sec * 1000));

        (*job->args)[1].Reset(isolate, statsTemp.getObject(context));
    }
    callback(job);
}

void File::getStatsAsync(const char *path, callbackJob *job, void (*callback)(callbackJob *j))
{
    std::thread thread(statsAsync, path, job, callback);
    thread.detach();
}

void File::getStatsSync(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::Local<v8::String> v8Path = args[0].As<v8::String>();

    const char *path = StaticHelpers::ToUtf8String(isolate, v8Path);

    struct stat buffer;
    int ret = stat(path, &buffer);
    if (ret != 0)
    {
        // TO DO
    }
    else
    {

        ObjectCreator statsTemp = ObjectCreator(isolate, "statsTemp");
        statsTemp.SetPropertyValue("mode", StaticHelpers::ToLocalString(isolate, getPermissions(buffer.st_mode).c_str()));
        statsTemp.SetPropertyValue("dev", v8::Integer::New(isolate, buffer.st_dev));
        statsTemp.SetPropertyValue("ino", v8::Integer::New(isolate, buffer.st_ino));
        statsTemp.SetPropertyValue("nlink", v8::Integer::New(isolate, buffer.st_nlink));
        statsTemp.SetPropertyValue("uid", v8::Integer::New(isolate, buffer.st_uid));
        statsTemp.SetPropertyValue("gid", v8::Integer::New(isolate, buffer.st_gid));
        statsTemp.SetPropertyValue("rdev", v8::Integer::New(isolate, buffer.st_rdev));
        statsTemp.SetPropertyValue("size", v8::Integer::New(isolate, buffer.st_size));
        statsTemp.SetPropertyValue("blksize", v8::Integer::New(isolate, buffer.st_blksize));
        statsTemp.SetPropertyValue("atimeMs", v8::Integer::New(isolate, buffer.st_atime * 1000));
        statsTemp.SetPropertyValue("mtimeMs", v8::Integer::New(isolate, buffer.st_mtime * 1000));
        statsTemp.SetPropertyValue("ctimeMs", v8::Integer::New(isolate, buffer.st_ctime * 1000));
        args.GetReturnValue().Set(statsTemp.getObject(context));
    }
}
