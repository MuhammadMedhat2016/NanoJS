#include "FileSystem.hpp"

EventLoop *File::loop;
extern msgQueue mqu;

void readAsync(const char *path, void *data, callbackJob *job)
{

    auto isolate = File::loop->isolate;
    auto context = job->context->Get(isolate);

    int64_t offset, length, position;
    (*job->additionalData)[0].Get(isolate)->IntegerValue(context).To(&offset);
    (*job->additionalData)[1].Get(isolate)->IntegerValue(context).To(&length);
    (*job->additionalData)[2].Get(isolate)->IntegerValue(context).To(&position);

    data = (char *)data + offset;
    std::ifstream myfile(path, std::ios::binary);
    myfile.seekg(position, std::ios::beg);
    myfile.read((char *)data, length);
    myfile.close();
    std::streamsize bytesRead = myfile.gcount();

    job->args = new std::vector<v8::Persistent<v8::Value>>(2);
    (*job->args)[0].Reset(isolate, v8::Null(isolate));
    (*job->args)[1].Reset(isolate, v8::Integer::New(isolate, bytesRead));

    File::loop->addCallbackJob(job);
}
void File::readFileAsync(const FunctionCallbackInfo<Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();

    v8::Local<Value> v8Path = args[0].As<v8::String>();
    v8::Local<v8::Uint8Array> array = args[1].As<v8::Uint8Array>();
    v8::Local<v8::Object> options = args[2].As<v8::Object>();
    v8::Local<v8::Function> callback = args[3].As<v8::Function>();

    v8::Local<v8::Integer> v8Offset = options->Get(context, v8::String::NewFromUtf8(isolate, "offset").ToLocalChecked()).ToLocalChecked().As<v8::Integer>();
    v8::Local<v8::Integer> v8Length = options->Get(context, v8::String::NewFromUtf8(isolate, "length").ToLocalChecked()).ToLocalChecked().As<v8::Integer>();
    v8::Local<v8::Integer> v8Position = options->Get(context, v8::String::NewFromUtf8(isolate, "position").ToLocalChecked()).ToLocalChecked().As<v8::Integer>();

    const char *path = StaticHelpers::ToUtf8String(isolate, v8Path);

    void *data = array->Buffer()->GetBackingStore()->Data();

    callbackJob *job = new callbackJob();

    job->func = new v8::Persistent<v8::Function>();
    job->context = new v8::Persistent<v8::Context>();

    job->func->Reset(isolate, callback);
    job->context->Reset(isolate, context);

    job->additionalData = new std::vector<v8::Persistent<v8::Value>>(3);
    (*job->additionalData)[0].Reset(isolate, v8Offset);
    (*job->additionalData)[1].Reset(isolate, v8Length);
    (*job->additionalData)[2].Reset(isolate, v8Position);

    File::loop->registerJob();
    // vec.push_back(std::async(std::launch::async, readAsync, path, job));
    std::thread thread = std::thread(readAsync, path, data, job);
    thread.detach();
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

void writeAsync(const char *path, void *data, int length, bool mode, callbackJob *job)
{
    auto isolate = File::loop->isolate;

    std::ofstream myfile;
    if (mode)
        myfile.open(path, std::ios::binary);
    else
        myfile.open(path, std::ios::app);

    std::streampos start_pos = myfile.tellp();
    myfile.write(reinterpret_cast<char *>(data), length);
    std::streampos end_pos = myfile.tellp();
    myfile.close();

    int bytesWritten = end_pos - start_pos;
    job->args = new std::vector<v8::Persistent<Value>>(2);
    (*job->args)[0].Reset(isolate, v8::Null(isolate));
    (*job->args)[1].Reset(isolate, v8::Integer::New(isolate, bytesWritten));
    File::loop->addCallbackJob(job);
}

void File::writeFileAsync(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = v8::Context::New(isolate);
    v8::Local<v8::String> v8Path = args[0].As<v8::String>();
    v8::Local<v8::Uint8Array> v8Data = args[1].As<v8::Uint8Array>();
    v8::Local<v8::String> v8Flag = args[2].As<v8::String>();
    v8::Local<v8::Function> callback = args[3].As<v8::Function>();

    const char *path = StaticHelpers::ToUtf8String(isolate, v8Path);

    callbackJob *job = new callbackJob();

    job->func = new v8::Persistent<v8::Function>();
    job->context = new v8::Persistent<v8::Context>();

    job->func->Reset(isolate, callback);
    job->context->Reset(isolate, context);

    void *data = v8Data->Buffer()->GetBackingStore()->Data();
    int64_t byteLength;
    v8Data->Get(context, StaticHelpers::ToLocalString(isolate, "byteLength")).ToLocalChecked().As<v8::Integer>()->IntegerValue(context).To(&byteLength);
    const char *flag = StaticHelpers::ToUtf8String(isolate, v8Flag);
    bool mode = true;
    if (flag[0] == 'w')
        mode = true;
    else
        mode = false;
    File::loop->registerJob();
    std::thread thread = std::thread(writeAsync, path, data, byteLength, mode, job);
    thread.detach();
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
v8::Local<v8::Object> buildStatsObject(struct stat &buffer)
{

    auto isolate = File::loop->isolate;

    auto context = isolate->GetCurrentContext();
    ObjectCreator statsTemp(isolate, "statsObject");

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

    return statsTemp.getObject(context);
}
void statsAsync(const char *path, callbackJob *job)
{
    auto isolate = File::loop->isolate;
    struct stat buffer;
    int ret = stat(path, &buffer);
    message *msg = new message();
    objectProject *object = new objectProject();
    if (ret != 0)
    {
        // TO DO
    }
    else
    {

        object->property.push_back({"mode", "string"});
        value *mode = new value();
        mode->setValue<std::string>(getPermissions(buffer.st_mode));
        object->values.push_back(mode);

        object->property.push_back({"size", "integer"});
        value *size = new value();
        size->setValue<int>(buffer.st_size);
        object->values.push_back(size);

        object->property.push_back({"dev", "integer"});
        value *dev = new value();
        dev->setValue<int>(buffer.st_dev);
        object->values.push_back(dev);

        object->property.push_back({"ino", "integer"});
        value *ino = new value();
        ino->setValue<int>(buffer.st_ino);
        object->values.push_back(ino);

        object->property.push_back({"nlink", "integer"});
        value *nlink = new value();
        nlink->setValue<int>(buffer.st_nlink);
        object->values.push_back(nlink);

        object->property.push_back({"uid", "integer"});
        value *uid = new value();
        uid->setValue<int>(buffer.st_uid);
        object->values.push_back(uid);

        object->property.push_back({"gid", "integer"});
        value *gid = new value();
        gid->setValue<int>(buffer.st_gid);
        object->values.push_back(gid);

        object->property.push_back({"rdev", "integer"});
        value *rdev = new value();
        rdev->setValue<int>(buffer.st_rdev);
        object->values.push_back(rdev);

        object->property.push_back({"blksize", "integer"});
        value *blksize = new value();
        blksize->setValue<int>(buffer.st_blksize);
        object->values.push_back(blksize);
    }
    msg->job = job;
    msg->objects->push_back(object);
    File::loop->mQu->addMsgJob(msg);
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
    job->context->Reset(isolate, context);

    File::loop->registerJob();
    std::thread thread(statsAsync, path, job);
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
        ObjectCreator *statsTemp = new ObjectCreator(isolate, "statsTemp");
        args.GetReturnValue().Set(buildStatsObject(buffer));
    }
}

void cpyJob(callbackJob *srcJob, callbackJob *destJob)
{
    auto isolate = File::loop->isolate;
    destJob->func = new v8::Persistent<v8::Function>();
    destJob->context = new v8::Persistent<v8::Context>();

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
void watcher(const char *path, callbackJob *job, u_int32_t pollTimeInterval, uint32_t* shouldStop)
{
    auto isolate = File::loop->isolate;
    std::ifstream in1(path);
    struct stat buffer;
    int ret = stat(path, &buffer);
    int prevSz = buffer.st_size;
    char *prevContent = new char[prevSz];
    in1.read(prevContent, prevSz);
    in1.close();
    while (!(*shouldStop))
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

            callbackJob *localJob = new callbackJob();

            cpyJob(job, localJob);

            File::loop->registerJob();

            message *msg = new message();

            msg->job = localJob;

            objectProject *object = new objectProject();

            object->property.push_back({"eventType", "string"});
            value *eventType = new value();
            eventType->setValue<std::string>("change");
            object->values.push_back(eventType);

            object->property.push_back({"File", "string"});
            value *file = new value();
            file->setValue<std::string>(std::string(path));
            object->values.push_back(file);

            msg->objects->push_back(object);
            File::loop->mQu->addMsgJob(msg);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(pollTimeInterval));
    }
    File::loop->withdrawJob();
    printf("watching stopped\n");
}

void File::watchFile(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    v8::Local<v8::String> v8Path = args[0].As<v8::String>();
    v8::Local<v8::Object> options = args[1].As<v8::Object>();
    v8::Local<v8::Function> callback = args[2].As<v8::Function>();

    const char *path = StaticHelpers::ToUtf8String(isolate, v8Path);

    callbackJob *job = new callbackJob();
    job->func = new v8::Persistent<v8::Function>();
    job->context = new v8::Persistent<v8::Context>();

    job->context->Reset(isolate, isolate->GetCurrentContext());
    job->func->Reset(isolate, callback);
    v8::Local<v8::Value> interval = options->Get(isolate->GetCurrentContext(), StaticHelpers::ToLocalString(isolate, "interval")).ToLocalChecked();
    uint32_t pollTimeInterval = interval.As<v8::Integer>()->Value();

    uint32_t *ptr = (uint32_t*)calloc(1, sizeof(uint32_t));
    std::unique_ptr<v8::BackingStore> store = v8::ArrayBuffer::NewBackingStore(
        ptr, sizeof(*ptr), [](void *, size_t, void *) {}, nullptr);
    v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(isolate, std::move(store));

    File::loop->registerJob();
    
    std::thread thread(watcher, path, job, pollTimeInterval, (uint32_t*)ptr);
    thread.detach();

    args.GetReturnValue().Set(v8::Uint32Array::New(buffer, 0, 1));
}
