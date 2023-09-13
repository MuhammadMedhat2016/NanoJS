#include "StaticHelpers.hpp"
const char *StaticHelpers::ToCString(const String::Utf8Value &value)
{
	return *value ? *value : "<string conversion failed>";
}
v8::Local<v8::String> StaticHelpers::ToLocalString(v8::Isolate *isolate, const char *string)
{
	return v8::String::NewFromUtf8(isolate, string).ToLocalChecked();
}

const char *StaticHelpers::ToUtf8String(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
	v8::Local<v8::String> v8String = value.As<v8::String>();
	int length = v8String->Utf8Length(isolate);
	char *cString = new char[length + 1];
	cString[length] = '\0';
	int flags = String::HINT_MANY_WRITES_EXPECTED |
				String::NO_NULL_TERMINATION |
				String::REPLACE_INVALID_UTF8;
			
	v8String->WriteUtf8(isolate, cString, length, nullptr, flags);

	return cString;
}
