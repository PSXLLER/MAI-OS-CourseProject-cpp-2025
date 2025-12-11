#pragma once
#include <cstddef>

static const size_t ALIGN = 16;
static const size_t ARENA_SIZE = 1024 * 1024 * 8; 

inline size_t align_size(size_t size)
{
    return (size + (ALIGN - 1)) & ~(ALIGN - 1);
}

void* alloc_arena(size_t size);
