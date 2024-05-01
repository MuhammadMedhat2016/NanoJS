#ifndef APP
#define APP

#include "../engine/Lemon.hpp"
#include <thread>


using v8::Context;


class App : public Lemon
{

public:
	static v8::ArrayBuffer::Allocator *allocator;
	static EventLoop *loop;
	static v8::Persistent<v8::Object> Binder;
	static v8::Persistent<v8::Object> FileSystem;
	static v8::Persistent<v8::Object> Buffer;
	static void setUint8ArrayPrototype(v8::Local<v8::Object> &object);
	static void log(const FunctionCallbackInfo<Value> &args);
	static void internalBinding(const FunctionCallbackInfo<Value> &args);
	static void logObject(int indentLevel, v8::Local<v8::Context> context, v8::Local<v8::Object> &obj);
	
	void setBinderObject();
	void addPropertyToBinder(const char *propertyName, v8::Local<v8::Value> propertyValue);
	void setupFileSystemModuleObject();
	void setupBufferModuleObject();

	void Start(int argc, char *argv[]);
	void SetupEnvironment();
};

#endif