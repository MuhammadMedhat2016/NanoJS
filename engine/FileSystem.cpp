#include "FileSystem.hpp"

EventLoop *File::loop;

void readAsync(const char *path, callbackJob *job, void (*RegisterCallback)(callbackJob *j))
{
    auto isolate = File::loop->isolate;

    std::ifstream myfile(path, std::ios::in);
    char *buffer = new char[1024];
    myfile.read(buffer, 1024);

    (*job->args)[0].Reset(isolate, v8::Undefined(isolate));
    (*job->args)[1].Reset(isolate, StaticHelpers::ToLocalString(isolate, buffer));

    RegisterCallback(job);
    myfile.close();
}

void writeAsync(const char *path, const char *data, callbackJob *job, void (*callback)(callbackJob *j))
{
    auto isolate = File::loop->isolate;
    std::ofstream myfile(path);
    myfile << data;
    char *bytesWritten = new char(20);
    bytesWritten = std::strcpy(bytesWritten, std::to_string(std::strlen(data)).c_str());
    (*job->args)[0].Reset(isolate, v8::Undefined(isolate));
    (*job->args)[1].Reset(isolate, StaticHelpers::ToLocalString(isolate, bytesWritten));
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
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();

    v8::Local<Value> v8Path = args[0];
    v8::Local<v8::Function> callback = args[1].As<v8::Function>();
    const char *path = StaticHelpers::ToString(args.GetIsolate(), v8Path);
    callbackJob *job = new callbackJob();
    job->func.Reset(args.GetIsolate(), callback);
    job->args = new std::vector<v8::Persistent<v8::Value>>(2);
    job->context.Reset(isolate, context);
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
    job->args = new std::vector<v8::Persistent<Value>>(2);
    job->context.Reset(isolate, context);

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

    callbackJob *job = new callbackJob();

    job->func.Reset(isolate, callback);
    job->args = new std::vector<v8::Persistent<v8::Value>>(2);
    job->context.Reset(isolate, context);
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
    auto context = job->context.Get(isolate);
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

    const char *path = StaticHelpers::ToString(isolate, v8Path);
    
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
