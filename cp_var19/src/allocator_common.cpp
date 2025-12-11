#include "allocator_common.h"
#include <sys/mman.h>
#include <unistd.h>

void* alloc_arena(size_t size)
{
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,-1, 0);
    return (ptr == MAP_FAILED) ? nullptr : ptr;
}
