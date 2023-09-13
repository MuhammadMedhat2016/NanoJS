#include "App.hpp"
#include <stdio.h>

EventLoop *App::loop = nullptr;

v8::Persistent<v8::Object> App::Binder;
v8::Persistent<v8::Object> App::FileSystem;
v8::Persistent<v8::Object> App::Buffer;
v8::ArrayBuffer::Allocator *App::allocator;

// void getZeroField(const v8::FunctionCallbackInfo<v8::Value> &args);

bool isAnyArrayBuffer(v8::Local<v8::Value> object)
{
	if (object->IsArrayBuffer() || object->IsArrayBufferView() || object->IsSharedArrayBuffer())
		return true;
	return false;
}

void getZeroField(const v8::FunctionCallbackInfo<v8::Value> &args)
{
	NestArrayBufferAllocator *allocator = reinterpret_cast<NestArrayBufferAllocator *>(App::allocator);

	uint32_t *field = allocator->zero_fill_field();
	std::unique_ptr<v8::BackingStore> backing =
		ArrayBuffer::NewBackingStore(
			field,
			sizeof(*field),
			[](void *, size_t, void *) {},
			nullptr);
	v8::Local<v8::ArrayBuffer> arrayBuffer = v8::ArrayBuffer::New(App::loop->isolate, std::move(backing));
	args.GetReturnValue().Set(v8::Uint32Array::New(arrayBuffer, 0, 1));
}

void App::SetupEnvironment()
{
	auto isolate = this->GetIsolate();
	App::allocator = this->getArrayBufferAllocator();
	App::loop = new EventLoop(this->GetIsolate());
	File::loop = App::loop;
	Timers::loop = App::loop;
	FileWatcher::loop = App::loop;
	Buffer::loop = App::loop;
	Buffer::global = this->GetGlobal();
	Buffer::ctx = this->context;

	this->GetGlobal()->Set(isolate, "log", FunctionTemplate::New(isolate, log));
	this->GetGlobal()->Set(isolate, "internalBinding", FunctionTemplate::New(isolate, internalBinding));
	this->GetGlobal()->Set(isolate, "setTimeOut", FunctionTemplate::New(isolate, Timers::setTimeOut));
	this->GetGlobal()->Set(isolate, "setInterval", FunctionTemplate::New(isolate, Timers::setInterval));

	setBinderObject();
	setupFileSystemModuleObject();
	setupBufferModuleObject();

	v8::Local<v8::Context> ctx = Binder.Get(isolate)->CreationContext();

	addPropertyToBinder("getZeroFillToggle", FunctionTemplate::New(isolate, getZeroField)->GetFunction(ctx).ToLocalChecked());

	// addPropertyToBinder("fs", FunctionTemplate::New(isolate, Environment::Test)->GetFunction(Binder.Get(isolate)->CreationContext()).ToLocalChecked());
}

void App::setBinderObject()
{
	auto isolate = this->GetIsolate();
	v8::Local<v8::Context> context = v8::Context::New(isolate);
	v8::Context::Scope HandleScope(context);
	Binder.Reset(isolate, v8::Object::New(isolate));
}
void App::addPropertyToBinder(const char *propertyName, v8::Local<v8::Value> propertyValue)
{
	auto isolate = this->GetIsolate();
	v8::Local<v8::Object> binder = Binder.Get(isolate);
	binder->Set(binder->CreationContext(), StaticHelpers::ToLocalString(isolate, propertyName), propertyValue);
}

void App::setupFileSystemModuleObject()
{

	auto isolate = this->GetIsolate();
	FunctionCreator readFileSync = FunctionCreator(isolate, "readFileSync", File::readFileSync);
	FunctionCreator readFileAsync = FunctionCreator(isolate, "readFileAsync", File::readFileAsync);
	FunctionCreator writeFileSync = FunctionCreator(isolate, "writeFileSync", File::writeFileSync);
	FunctionCreator writeFileAsync = FunctionCreator(isolate, "writeFileAsync", File::writeFileAsync);
	FunctionCreator getStatsAsync = FunctionCreator(isolate, "getStatsAsync", File::getStatsAsync);
	FunctionCreator getStatsSync = FunctionCreator(isolate, "getStatsSync", File::getStatsSync);
	FunctionCreator watch = FunctionCreator(isolate, "watch", FileWatcher::watch);

	v8::Local<v8::Context> context = Binder.Get(isolate)->CreationContext();
	v8::Context::Scope handleScope(context);
	v8::Local<v8 ::Object> fsObject = v8::Object::New(isolate);

	readFileSync.attachMethodToObject(fsObject);
	readFileAsync.attachMethodToObject(fsObject);
	writeFileSync.attachMethodToObject(fsObject);
	writeFileAsync.attachMethodToObject(fsObject);
	getStatsAsync.attachMethodToObject(fsObject);
	getStatsSync.attachMethodToObject(fsObject);
	watch.attachMethodToObject(fsObject);

	FileSystem.Reset(isolate, fsObject);
	addPropertyToBinder("fs", FileSystem.Get(isolate));
}
void App::setupBufferModuleObject()
{
	auto isolate = this->GetIsolate();

	FunctionCreator byteLengthUtf8 = FunctionCreator(isolate, "byteLengthUtf8", Buffer::byteLengthUtf8);
	FunctionCreator utf8Write = FunctionCreator(isolate, "utf8Write", Buffer::utf8Write);
	FunctionCreator ucs2Write = FunctionCreator(isolate, "ucs2Write", Buffer::ucs2Write);
	FunctionCreator fromUtf8 = FunctionCreator(isolate, "fromUtf8", Buffer::fromUtf8);
	FunctionCreator fromUtf16 = FunctionCreator(isolate, "fromUtf16", Buffer::fromUtf16);
	FunctionCreator fill = FunctionCreator(isolate, "fill", Buffer::fill);

	v8::Local<v8::Context> context = Binder.Get(isolate)->CreationContext();
	v8::Context::Scope handleScope(context);
	v8::Local<v8 ::Object> bufferObj = v8::Object::New(isolate);

	byteLengthUtf8.attachMethodToObject(bufferObj);
	utf8Write.attachMethodToObject(bufferObj);
	ucs2Write.attachMethodToObject(bufferObj);
	fromUtf8.attachMethodToObject(bufferObj);
	fromUtf16.attachMethodToObject(bufferObj);
	fill.attachMethodToObject(bufferObj);

	Buffer.Reset(isolate, bufferObj);
	addPropertyToBinder("Buffer", Buffer.Get(isolate));
}

void App::logObject(int indentLevel, v8::Local<v8::Context> context, v8::Local<v8::Object> &obj)
{
	auto isolate = context->GetIsolate();
	v8::Local<v8::Array> properties = obj->GetPropertyNames(context).ToLocalChecked();
	std::string indent = "";
	for (int i = 0; i < indentLevel; ++i)
		indent += " ";
	for (int i = 0; i < properties->Length(); ++i)
	{

		v8::Local<v8::String> propoerty = properties->Get(context, i).ToLocalChecked().As<v8::String>();
		// printf("%s\n", StaticHelpers::ToUtf8String(isolate,propoerty));
		v8::Local<v8::Value> value = obj->Get(context, propoerty).ToLocalChecked();

		if (value->IsFunction())
		{
			printf("%s : %s\n", (indent + StaticHelpers::ToUtf8String(isolate, propoerty)).c_str(), " => Native Code");
		}
		else if (value->IsObject())
		{
			printf("%s : %s\n", (indent + StaticHelpers::ToUtf8String(isolate, propoerty)).c_str(), "{");
			v8::Local<v8::Object> obj = value.As<v8::Object>();
			logObject(indentLevel + 2, context, obj);
			printf("%s\n", (indent + "}").c_str());
		}
		else
		{
			printf("%s : %s \n", (indent + StaticHelpers::ToUtf8String(isolate, propoerty)).c_str(), StaticHelpers::ToUtf8String(isolate, value));
		}
	}
}
void logBuffer(char *ptr, int byteLength)
{
	printf("< Buffer ");
	for (int i = 0; i < byteLength; ++i, ptr++)
		printf("%.2x ", *ptr & 255);
	printf(">");
}
void App::log(const FunctionCallbackInfo<Value> &args)
{
	auto isolate = args.GetIsolate();
	auto context = isolate->GetCurrentContext();
	for (int i = 0; i < args.Length(); ++i)
	{

		if (args[i]->IsFunction())
		{
			printf("%s %s\n", StaticHelpers::ToUtf8String(isolate, Local<v8::Function>::Cast(args[i])->GetName()), " => native code");
		}
		else if (isAnyArrayBuffer(args[i]))
		{
			char *data = nullptr;
			size_t bufferLength;
			size_t offset = 0;
			if (args[i]->IsArrayBufferView())
			{
				v8::Local<v8::ArrayBufferView> array = v8::Local<v8::ArrayBufferView>::Cast(args[i]);
				data = reinterpret_cast<char *>(array->Buffer()->GetBackingStore()->Data());
				bufferLength = array->ByteLength();
				data = data + array->ByteOffset();
			}
			else if (args[i]->IsArrayBuffer())
			{
				v8::Local<v8::ArrayBuffer> array = v8::Local<v8::ArrayBuffer>::Cast(args[i]);
				data = reinterpret_cast<char *>(array->GetBackingStore()->Data());
				bufferLength = array->ByteLength();
			}
			else
			{
				v8::Local<v8::SharedArrayBuffer> array = v8::Local<v8::SharedArrayBuffer>::Cast(args[i]);
				data = reinterpret_cast<char *>(array->GetBackingStore()->Data());
				bufferLength = array->ByteLength();
			}
			logBuffer(data, bufferLength);
		}
		else if (args[i]->IsObject())
		{
			v8::Local<v8::Object> obj = args[i].As<v8::Object>();
			printf("%s\n", "{");
			App::logObject(3, context, obj);
			printf("%s\n", "}");
		}
		else
		{

			if (args[i]->IsNumber())
			{
				int64_t num = 0;
				args[i]->IntegerValue(context).To(&num);
				printf("%d ", num);
			}
			else if (args[i]->IsBoolean())
			{
				bool bol = args[i]->BooleanValue(isolate);
				if (bol)
					printf("%d ", bol);
				else
					printf("%d ", bol);
			}

			else
			{
				v8::Local<v8::String> v8String = args[i].As<v8::String>();
				const char *str = StaticHelpers::ToUtf8String(isolate, v8String);
				printf("%s ", str);
			}
		}
	}
	printf("\n");
}

void App::internalBinding(const FunctionCallbackInfo<Value> &args)
{
	v8::Local<v8::String> propertyName = args[0].As<v8::String>();
	v8::Local<v8::Object> binder = Binder.Get(args.GetIsolate());
	v8::Local<v8::Value> value = binder->Get(binder->CreationContext(), propertyName).ToLocalChecked();
	args.GetReturnValue().Set(value);
}


void App::Start(int argc, char *argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		// Get filename of the javascript file to run
		const char *filename = argv[i];

		// Create a new context for executing javascript code
		Local<Context> context = this->CreateLocalContext();

		// Enter the new context
		Context::Scope contextscope(context);
		// Run the javascript file
		this->RunJsFromFile(filename);
	}
}
