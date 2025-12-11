#include "freelist_allocator.h"
#include "allocator_common.h"
#include <iostream>
#include <cstring>

static Block* free_list = nullptr;
static void* arena_start = nullptr;

size_t fl_global_split_count = 0;
size_t fl_global_merge_count = 0;

void fl_init()
{
    arena_start = alloc_arena(ARENA_SIZE);
    if (!arena_start)
    {
        std::cerr << "FreeList: mmap failed\n";
        return;
    }

    Block* first = (Block*)arena_start;
    first->size = ARENA_SIZE - sizeof(Block);
    first->free = true;
    first->next = nullptr;
    first->prev = nullptr;

    free_list = first;

    fl_global_split_count = 0;
    fl_global_merge_count = 0;
}

static void split_block(Block* block, size_t size)
{
    if (block->size < size + sizeof(Block) + ALIGN)
        return;

    fl_global_split_count++;

    char* raw = (char*)block;
    Block* new_block = (Block*)(raw + sizeof(Block) + size);

    new_block->size = block->size - size - sizeof(Block);
    new_block->free = true;
    new_block->next = block->next;
    new_block->prev = block;

    if (block->next)
        block->next->prev = new_block;

    block->next = new_block;
    block->size = size;
}

static Block* find_block(size_t size)
{
    Block* cur = free_list;
    while (cur)
    {
        if (cur->free && cur->size >= size)
            return cur;
        cur = cur->next;
    }
    return nullptr;
}

void* fl_malloc(size_t size)
{
    size = align_size(size);

    Block* block = find_block(size);
    if (!block)
        return nullptr;

    split_block(block, size);

    block->free = false;
    return (char*)block + sizeof(Block);
}

static void merge_blocks(Block* block)
{
    if (block->next && block->next->free)
    {
        fl_global_merge_count++;

        block->size += sizeof(Block) + block->next->size;

        Block* next2 = block->next->next;
        block->next = next2;
        if (next2)
            next2->prev = block;
    }

    if (block->prev && block->prev->free)
    {
        fl_global_merge_count++;

        block->prev->size += sizeof(Block) + block->size;

        Block* next2 = block->next;
        block->prev->next = next2;
        if (next2)
            next2->prev = block->prev;
    }
}

void fl_free(void* ptr)
{
    if (!ptr) return;

    Block* b = (Block*)((char*)ptr - sizeof(Block));
    b->free = true;

    merge_blocks(b);
}

void* fl_realloc(void* ptr, size_t new_size)
{
    if (!ptr)
        return fl_malloc(new_size);

    Block* b = (Block*)((char*)ptr - sizeof(Block));
    if (b->size >= new_size)
        return ptr;

    void* new_ptr = fl_malloc(new_size);
    if (!new_ptr)
        return nullptr;

    memcpy(new_ptr, ptr, b->size);
    fl_free(ptr);
    return new_ptr;
}

Block* fl_get_free_list_head()
{
    return free_list;
}

size_t fl_get_real_block_size(void* ptr)
{
    Block* b = (Block*)((char*)ptr - sizeof(Block));
    return b->size;
}

void dump_free_list()
{
    std::cout << "[Free list dump]\n";

    Block* cur = free_list;
    while (cur)
    {
        std::cout << "  Block@" << cur << " size=" << cur->size << " free=" << cur->free << "\n";

        cur = cur->next;
    }
}
