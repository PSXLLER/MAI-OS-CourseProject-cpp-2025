#include <iostream>
#include "freelist_allocator.h"
#include "pow2_allocator.h"
#include "allocator_common.h"

int main()
{
    std::cout << "=== DEMO: FreeList & Power-of-Two Allocators ===\n\n";

    std::cout << "[INIT] Initializing FreeList allocator...\n";
    fl_init();

    std::cout << "[INIT] Initializing Power-of-Two allocator...\n";
    pow2_init();

    std::cout << "\n=== DEMO: FreeList Allocator ===\n";

    void* a1 = fl_malloc(32);
    void* a2 = fl_malloc(64);
    void* a3 = fl_malloc(128);

    std::cout << "Allocated blocks:\n";
    std::cout << " a1 = " << a1 << "\n";
    std::cout << " a2 = " << a2 << "\n";
    std::cout << " a3 = " << a3 << "\n";

    std::cout << "\n[STATE] FreeList after allocations:\n";
    dump_free_list();

    std::cout << "\n[FREE] Freeing a2...\n";
    fl_free(a2);
    dump_free_list();

    std::cout << "\n[FREE] Freeing a1 and a3...\n";
    fl_free(a1);
    fl_free(a3);
    dump_free_list();


    std::cout << "\n=== DEMO: Power-of-Two Allocator ===\n";

    void* b1 = pow2_malloc(32);
    void* b2 = pow2_malloc(128);
    void* b3 = pow2_malloc(300);

    std::cout << "Allocated blocks:\n";
    std::cout << " b1 = " << b1 << "\n";
    std::cout << " b2 = " << b2 << "\n";
    std::cout << " b3 = " << b3 << "\n";

    std::cout << "\n[STATE] Power-of-Two after allocations:\n";
    pow2_dump();

    std::cout << "\n[FREE] Freeing b2...\n";
    pow2_free(b2);
    pow2_dump();

    std::cout << "\n[FREE] Freeing b1 and b3...\n";
    pow2_free(b1);
    pow2_free(b3);
    pow2_dump();


    std::cout << "\n=== DEMO FINISHED SUCCESSFULLY ===\n";
    return 0;
}
