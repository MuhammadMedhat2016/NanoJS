#include "NestArrayBufferAllocator.hpp"

#define LIKELY(expr) __builtin_expect(!!(expr), 1)
#define UNLIKELY(expr) __builtin_expect(!!(expr), 0)

void *NestArrayBufferAllocator::Allocate(size_t size)
{   
    void *ret;

    if (zero_fill_field_)
        ret = allocator_->Allocate(size);
    else
        ret = allocator_->AllocateUninitialized(size);
    if (LIKELY(ret != nullptr))
        total_mem_usage_.fetch_add(size, std::memory_order_relaxed);
    return ret;
}

void *NestArrayBufferAllocator::AllocateUninitialized(size_t size)
{

    void *ret = allocator_->AllocateUninitialized(size);
    if (LIKELY(ret != nullptr))
        total_mem_usage_.fetch_add(size, std::memory_order_relaxed);
    return ret;
}

void *NestArrayBufferAllocator::Reallocate(void *data, size_t old_size, size_t size)
{

    void *ret = allocator_->Reallocate(data, old_size, size);
    if (LIKELY(ret != nullptr) || UNLIKELY(size == 0))
        total_mem_usage_.fetch_add(size - old_size, std::memory_order_relaxed);
    return ret;
}
void NestArrayBufferAllocator::Free(void *data, size_t size)
{

    total_mem_usage_.fetch_sub(size, std::memory_order_relaxed);
    allocator_->Free(data, size);
}