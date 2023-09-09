#include "Buffer.hpp"

EventLoop *Buffer::loop;

void Buffer::byteLengthUtf8(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = loop->isolate;
    v8::Local<v8::String> str = args[0].As<v8::String>();
    v8::Local<v8::Integer> len = v8::Integer::New(isolate, str->Utf8Length(isolate));
    args.GetReturnValue().Set(len);
}

void Buffer::utf8Write(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Local<v8::Uint8Array> buffer = args[0].As<v8::Uint8Array>();
    v8::Local<v8::String> str = args[1].As<v8::String>();
    v8::Local<v8::Integer> offset = args[2].As<v8::Integer>();
    v8::Local<v8::Integer> length = args[3].As<v8::Integer>();

    v8::Local<v8::Integer> bytesWritten = v8::Integer::New(args.GetIsolate(), utf8Write(buffer, str, offset, length));
    args.GetReturnValue().Set(bytesWritten);
}

int Buffer::utf8Write(v8::Local<v8::Uint8Array> array, v8::Local<v8::String> str, v8::Local<v8::Integer> offset, v8::Local<v8::Integer> length)
{
    auto isolate = Buffer::loop->isolate;
    char *dest = reinterpret_cast<char *>(array->Buffer()->GetBackingStore()->Data());
    int flags = String::HINT_MANY_WRITES_EXPECTED |
                String::NO_NULL_TERMINATION |
                String::REPLACE_INVALID_UTF8;
    int64_t len = 0;
    length->IntegerValue(isolate->GetCurrentContext()).To(&len);
    return str->WriteUtf8(isolate, dest, len, nullptr);
}
