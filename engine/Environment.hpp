#ifndef ENVIRONMENT
#define ENVIRONMENT

#include <cassert>
#include "libplatform/libplatform.h"
#include <v8.h>
#include <memory>
#include "Buffers/NestArrayBufferAllocator.hpp"


using v8::ArrayBuffer;
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::ObjectTemplate;
using v8::Platform;
using v8::String;
using v8::V8;
using v8::Value;

class Environment
{

private:
	std::unique_ptr<Platform> platform;
	Isolate::CreateParams create_params;

	Isolate *isolate;
	Local<ObjectTemplate> global;

	static void Version(const FunctionCallbackInfo<Value> &args);

protected:
	virtual Local<Context> CreateLocalContext() final;
	virtual Local<ObjectTemplate> GetGlobal() final;
	v8::Local<v8::Context> context;

public:

	Isolate *GetIsolate();

	v8::ArrayBuffer::Allocator* getArrayBufferAllocator();

	virtual void CreatePlatform(char *argv[]) final;
	virtual void CreateVM() final;
	virtual void ShutdownVM() final;

	virtual void CreateGlobalEnvironment() final;

	virtual void SetupEngineEnvironment() final;
	virtual void SetupEnvironment();
};

#endif