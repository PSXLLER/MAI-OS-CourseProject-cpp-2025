#include "pow2_allocator.h"
#include "allocator_common.h"
#include <iostream>
#include <cstring>

struct P2Block
{
    size_t size;     
    P2Block* next;
};

static const size_t MAX_CLASSES = 32;
static void* arena = nullptr;

static P2Block* free_lists[MAX_CLASSES];

size_t pow2_global_split_count = 0;
size_t pow2_global_merge_count = 0; 

static size_t next_pow2(size_t x)
{
    size_t p = 1;
    while (p < x) p <<= 1;
    return p;
}

size_t size_to_class(size_t size)
{
    size_t cls = 0;
    while ((1ULL << cls) < size) cls++;
    return cls;
}

void pow2_init()
{
    arena = alloc_arena(ARENA_SIZE);
    if (!arena)
    {
        std::cerr << "pow2_init: mmap failed\n";
        return;
    }

    for (size_t i = 0; i < MAX_CLASSES; i++)
        free_lists[i] = nullptr;

    size_t max_cls = size_to_class(ARENA_SIZE);

    P2Block* b = (P2Block*)arena;
    b->size = (1ULL << max_cls);
    b->next = nullptr;

    free_lists[max_cls] = b;
}

static P2Block* split_block(size_t cls)
{
    size_t cur = cls;

    while (cur < MAX_CLASSES && free_lists[cur] == nullptr)
        cur++;

    if (cur == MAX_CLASSES)
        return nullptr;

    P2Block* block = free_lists[cur];
    free_lists[cur] = block->next;

    while (cur > cls)
    {
        cur--;

        size_t half = (1ULL << cur);

        P2Block* leftover = (P2Block*)((char*)block + half);
        leftover->size = half;
        leftover->next = free_lists[cur];
        free_lists[cur] = leftover;

        block->size = half;

        pow2_global_split_count++;
    }

    return block;
}

void* pow2_malloc(size_t size)
{
    if (size == 0) return nullptr;

    size_t need = next_pow2(size + sizeof(P2Block));
    size_t cls = size_to_class(need);

    if (cls >= MAX_CLASSES)
        return nullptr;

    if (free_lists[cls] != nullptr)
    {
        P2Block* b = free_lists[cls];
        free_lists[cls] = b->next;
        return (char*)b + sizeof(P2Block);
    }

    P2Block* b = split_block(cls);
    if (!b) return nullptr;

    return (char*)b + sizeof(P2Block);
}

void pow2_free(void* ptr)
{
    if (!ptr) return;

    P2Block* block = (P2Block*)((char*)ptr - sizeof(P2Block));
    size_t cls = size_to_class(block->size);

    block->next = free_lists[cls];
    free_lists[cls] = block;
}

bool pow2_class_nonempty(size_t cls)
{
    return free_lists[cls] != nullptr;
}

size_t pow2_blocks_in_class(size_t cls)
{
    size_t count = 0;
    P2Block* c = free_lists[cls];
    while (c)
    {
        count++;
        c = c->next;
    }
    return count;
}

size_t pow2_get_real_block_size(void* ptr)
{
    if (!ptr) return 0;

    P2Block* b = (P2Block*)((char*)ptr - sizeof(P2Block));
    return b->size;
}

void pow2_dump()
{
    std::cout << "\n[POWER-OF-TWO ALLOCATOR DUMP]\n";

    for (size_t c = 0; c < MAX_CLASSES; c++)
    {
        size_t size = (1ULL << c);
        std::cout << "Class " << c << " (size=" << size << "): ";

        P2Block* cur = free_lists[c];
        if (!cur)
        {
            std::cout << "empty\n";
            continue;
        }

        while (cur)
        {
            std::cout << "[" << cur << "] -> ";
            cur = cur->next;
        }
        std::cout << "NULL\n";
    }

    std::cout << "[END]\n\n";
}
