#include "StaticHelpers.hpp"
const char* StaticHelpers::ToCString(const String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}
v8::Local<v8::String> StaticHelpers::ToLocalString(v8::Isolate *isolate, const char *string)
{
	return v8::String::NewFromUtf8(isolate, string).ToLocalChecked();
}

const char* StaticHelpers::ToString(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
	v8::Local<v8::String> v8String = value->ToString(isolate->GetCurrentContext()).ToLocalChecked();
	int length = v8String->Length();
	char *cString = new char[length + 1];
	v8String->WriteUtf8(isolate, cString, length + 1);
	return cString;
}