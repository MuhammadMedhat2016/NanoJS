#include "App.hpp"

EventLoop *App::loop = nullptr;

v8::Persistent<v8::Object> App::Binder;
v8::Persistent<v8::Object> App::FileSystem;

void App::SetupEnvironment()
{
	App::loop = new EventLoop(this->GetIsolate());
	File::loop = App::loop;
	Timers::loop = App::loop;
	FileWatcher::loop = App::loop;
	auto isolate = this->GetIsolate();

	this->GetGlobal()->Set(isolate, "log", FunctionTemplate::New(isolate, log));
	this->GetGlobal()->Set(isolate, "internalBinding", FunctionTemplate::New(isolate, internalBinding));
	this->GetGlobal()->Set(isolate, "setTimeOut", FunctionTemplate::New(isolate, Timers::setTimeOut));
	this->GetGlobal()->Set(isolate, "setInterval", FunctionTemplate::New(isolate, Timers::setInterval));

	setBinderObject();
	setupFileSystemModuleObject();

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

void App::logObject(int indentLevel, v8::Local<v8::Context> context, v8::Local<v8::Object> obj)
{
	auto isolate = context->GetIsolate();
	v8::Local<v8::Array> properties = obj->GetPropertyNames(context).ToLocalChecked();
	std::string indent = "";
	for (int i = 0; i < indentLevel; ++i)
		indent += " ";
	for (int i = 0; i < properties->Length(); ++i)
	{
		v8::Local<v8::String> propoerty = properties->Get(context, i).ToLocalChecked().As<v8::String>();
		v8::Local<v8::Value> value = obj->Get(context, propoerty).ToLocalChecked();
		if (value->IsObject())
		{

			printf("%s : %s\n", (indent + StaticHelpers::ToString(isolate, propoerty)).c_str(), "{");
			logObject(indentLevel + 2, context, value.As<v8::Object>());
			printf("%s\n", (indent + "}").c_str());
		}
		else
		{
			printf("%s : %s \n", (indent + StaticHelpers::ToString(isolate, propoerty)).c_str(), StaticHelpers::ToString(isolate, value));
		}
	}
}
void App::log(const FunctionCallbackInfo<Value> &args)
{
	auto isolate = args.GetIsolate();
	auto context = isolate->GetCurrentContext();
	for (int i = 0; i < args.Length(); ++i)
	{
		if (args[i]->IsObject())
		{
			printf("%s\n", "{");
			App::logObject(3, context, args[i].As<v8::Object>());
			printf("%s\n", "}");
		}
		else
		{
			v8::Local<v8::String> v8String = args[i].As<v8::String>();
			const char *str = StaticHelpers::ToString(isolate, v8String);
			printf("%s ", str);
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