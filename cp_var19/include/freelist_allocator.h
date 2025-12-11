#pragma once
#include <cstddef>

struct Block
{
    size_t size;
    bool free;
    Block* next;
    Block* prev;
};

void fl_init();
void* fl_malloc(size_t size);
void  fl_free(void* ptr);
void* fl_realloc(void* ptr, size_t new_size);

Block* fl_get_free_list_head();
size_t fl_get_real_block_size(void* ptr);

extern size_t fl_global_split_count;
extern size_t fl_global_merge_count;

void dump_free_list();
