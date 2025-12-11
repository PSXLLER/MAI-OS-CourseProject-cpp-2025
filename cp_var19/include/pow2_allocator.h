#pragma once
#include <cstddef>

void pow2_init();
void* pow2_malloc(size_t size);
void  pow2_free(void* ptr);

bool   pow2_class_nonempty(size_t cls);
size_t pow2_blocks_in_class(size_t cls);
size_t pow2_get_real_block_size(void* ptr);

extern size_t pow2_global_split_count;
extern size_t pow2_global_merge_count;

void pow2_dump();
