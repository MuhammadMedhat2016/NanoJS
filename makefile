CXX = g++
V8 = engine/lib/v8

define INCLUDE
	$(V8)/include
	engine/Core.cpp
	engine/Environment.cpp
	engine/Lemon.cpp
	engine/StaticHelpers.cpp
	engine/ObjectCreator.cpp
	engine/EventLoop.cpp
	engine/FileSystem.cpp
	engine/Timers.cpp
	engine/FunctionCreator.cpp
	engine/Buffers/NestArrayBufferAllocator.cpp
	engine/lib/v8/include/v8.h
	engine/Buffer.cpp
endef

define APP
	app/*.cpp
endef

define LIB
	$(V8)/out/x64.release/obj/
endef

define OBJ
	v8_monolith
endef

export INCLUDE
export APP
export LIB
export OBJ

build:
	$(CXX) -I $$INCLUDE $$APP -L $$LIB -l $$OBJ -std=c++20 -pthread -o lemon