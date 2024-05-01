#pragma once

#include "../lib/v8/include/v8.h"
#include <atomic>
class NestArrayBufferAllocator;

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator
{
public:
    static std::unique_ptr<ArrayBufferAllocator> Create(
        bool always_debug = false);

private:
    virtual NestArrayBufferAllocator *GetImpl() = 0;
};

class NestArrayBufferAllocator : public ArrayBufferAllocator
{
public:
    inline uint32_t *zero_fill_field() { return &zero_fill_field_; }

    void *Allocate(size_t size) override; // Defined in src/node.cc
    void *AllocateUninitialized(size_t size) override;
    void Free(void *data, size_t size) override;
    void *Reallocate(void *data, size_t old_size, size_t size);
    virtual void RegisterPointer(void *data, size_t size)
    {
        total_mem_usage_.fetch_add(size, std::memory_order_relaxed);
    }
    virtual void UnregisterPointer(void *data, size_t size)
    {
        total_mem_usage_.fetch_sub(size, std::memory_order_relaxed);
    }

    NestArrayBufferAllocator *GetImpl() final { return this; }
    inline uint64_t total_mem_usage() const
    {
        return total_mem_usage_.load(std::memory_order_relaxed);
    }

private:
    uint32_t zero_fill_field_ = 1;
    std::atomic<size_t> total_mem_usage_{0};
    std::unique_ptr<v8::ArrayBuffer::Allocator> allocator_{v8::ArrayBuffer::Allocator::NewDefaultAllocator()};
};
