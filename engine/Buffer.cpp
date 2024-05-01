#include "Buffer.hpp"

EventLoop *Buffer::loop;
Local<ObjectTemplate> Buffer::global;
v8::Local<v8::Context> Buffer::ctx;

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
    printf("%d\n", buffer->IsUint8Array());
    int64_t offset = 0;
    args[2].As<v8::Integer>()->IntegerValue(args.GetIsolate()->GetCurrentContext()).To(&offset);
    v8::Local<v8::Integer> length = args[3].As<v8::Integer>();
    int64_t len = 0;
    length->IntegerValue(args.GetIsolate()->GetCurrentContext()).To(&len);
    v8::Local<v8::Integer> bytesWritten = v8::Integer::New(args.GetIsolate(), utf8Write(buffer, str, offset, len));
    args.GetReturnValue().Set(bytesWritten);
}

int Buffer::utf8Write(v8::Local<v8::Uint8Array> array, v8::Local<v8::String> str, int64_t offset, int64_t length)
{
    auto isolate = Buffer::loop->isolate;
    char *dest = reinterpret_cast<char *>(array->Buffer()->GetBackingStore()->Data());
    dest = dest + offset;
    int flags = String::HINT_MANY_WRITES_EXPECTED |
                String::NO_NULL_TERMINATION |
                String::REPLACE_INVALID_UTF8;

    return str->WriteUtf8(isolate, dest, length, nullptr, flags);
}
void Buffer::ucs2Write(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Local<v8::Uint8Array> buffer = args[0].As<v8::Uint8Array>();
    v8::Local<v8::String> str = args[1].As<v8::String>();

    int64_t offset = 0;
    args[2].As<v8::Integer>()->IntegerValue(args.GetIsolate()->GetCurrentContext()).To(&offset);
    v8::Local<v8::Integer> length = args[3].As<v8::Integer>();
    int64_t len = 0;
    length->IntegerValue(args.GetIsolate()->GetCurrentContext()).To(&len);
    v8::Local<v8::Integer> bytesWritten = v8::Integer::New(args.GetIsolate(), ucs2Write(buffer, str, offset, len) * 2);
    args.GetReturnValue().Set(bytesWritten);
}
int Buffer::ucs2Write(v8::Local<v8::Uint8Array> array, v8::Local<v8::String> str, int64_t offset, int64_t length)
{
    auto isolate = Buffer::loop->isolate;
    char *dest = reinterpret_cast<char *>(array->Buffer()->GetBackingStore()->Data());
    dest = dest + offset;
    uint16_t *buffer = reinterpret_cast<uint16_t *>(dest);

    int flags = String::HINT_MANY_WRITES_EXPECTED |
                String::NO_NULL_TERMINATION |
                String::REPLACE_INVALID_UTF8;

    return str->Write(isolate, buffer, 0, length);
}
void Buffer::fromUtf8(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::Local<v8::Uint8Array> array = args[0].As<v8::Uint8Array>();
    int64_t offset = 0, len;
    args[1].As<v8::Integer>()->IntegerValue(context).To(&offset);
    args[2].As<v8::Integer>()->IntegerValue(context).To(&len);
    char *data = reinterpret_cast<char *>(array->Buffer()->GetBackingStore()->Data());
    data = data + offset;
    v8::Local<v8::String> str = v8::String::NewFromUtf8(isolate, data, v8::NewStringType::kNormal, len).ToLocalChecked();

    args.GetReturnValue().Set(str);
}
void Buffer::fromUtf16(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::Local<v8::Uint8Array> array = args[0].As<v8::Uint8Array>();
    int64_t offset = 0, length;
    args[1].As<v8::Integer>()->IntegerValue(context).To(&offset);
    args[2].As<v8::Integer>()->IntegerValue(context).To(&length);
    char *data = reinterpret_cast<char *>(array->Buffer()->GetBackingStore()->Data());
    data = data + offset;
    v8::Local<v8::String> str = v8::String::NewFromTwoByte(isolate, reinterpret_cast<uint16_t *>(data), v8::NewStringType::kNormal, length / 2).ToLocalChecked();
    v8::String::Utf8Value utf(isolate, str);
    v8::Local<v8::String> ret = v8::String::NewFromUtf8(isolate, *utf).ToLocalChecked();
    args.GetReturnValue().Set(ret);
}
void Buffer::fill(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    v8::Local<v8::Uint8Array> buff = args[0].As<v8::Uint8Array>();
    v8::Local<v8::Integer> v8Offset = args[2].As<v8::Integer>();
    v8::Local<v8::Integer> v8End = args[3].As<v8::Integer>();

    int64_t offset;
    v8Offset->IntegerValue(context).To(&offset);
    int64_t end;
    v8End->IntegerValue(context).To(&end);

    if (args.Length() == 4)
    {
        v8::Local<v8::Integer> v8Value = args[1].As<v8::Integer>();

        int64_t value;
        v8Value->IntegerValue(context).To(&value);

        int8_t oneByteValue = value & 255;
        char *startAddress = reinterpret_cast<char *>(buff->Buffer()->GetBackingStore()->Data());
        memset(startAddress, oneByteValue, end);
        return;
    }
    v8::Local<v8::String> value = args[1].As<v8::String>();
    v8::Local<v8::Integer> v8EncodingVal = args[4].As<v8::Integer>();

    int64_t encodingVal;
    v8EncodingVal->IntegerValue(context).To(&encodingVal);

    if (encodingVal == 0)
    {
        int length = value->Utf8Length(isolate);
        char *buffer = new char[length];
        value->WriteUtf8(isolate, buffer, length);
        char *startAddress = reinterpret_cast<char *>(buff->Buffer()->GetBackingStore()->Data());
        int i = 0, j = 0;
        while (end--)
        {
            startAddress[i] = buffer[j];
            j++;
            i++;
            j = j % length;
        }
    }
    else
    {
        int length = value->Length();
        char *buffer = new char[length * 2];
        value->Write(isolate, reinterpret_cast<uint16_t *>(buffer), 0, length);
        char *startAddress = reinterpret_cast<char *>(buff->Buffer()->GetBackingStore()->Data());
        int i = 0, j = 0;
        while (end--)
        {
            startAddress[i] = buffer[j];
            j++;
            i++;
            j = j % (length * 2);
        }
    }
}
