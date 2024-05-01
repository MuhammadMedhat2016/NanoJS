#ifndef STATICHELPERS
#define STATICHELPERS

#include "libplatform/libplatform.h"
#include <v8.h>

using v8::String;

class StaticHelpers
{

public:
	static const char *ToCString(const String::Utf8Value &value);
	static v8::Local<v8::String> ToLocalString(v8::Isolate *isolate, const char *string);
	static const char *ToUtf8String(v8::Isolate *isolate, v8::Local<v8::Value> value);
	static const uint16_t* ToUcs2String(v8::Isolate *isolate, v8::Local<v8::Value> value);
};

#endif