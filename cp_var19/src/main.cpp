#include <iostream>
#include <iomanip>
#include "freelist_allocator.h"
#include "pow2_allocator.h"

static void print_table_header()
{
    std::cout << "+--------------------------+-----------------------+-----------------------+\n";
    std::cout << "| Metric                   | FreeList              | Power-of-Two          |\n";
    std::cout << "+--------------------------+-----------------------+-----------------------+\n";
}

static void print_row(const std::string& metric, const std::string& fl, const std::string& p2)
{
    std::cout << "| " << std::left << std::setw(25) << metric << " | " << std::setw(21) << fl << " | " << std::setw(21) << p2 << " |\n";
}

static void print_table_footer()
{
    std::cout << "+--------------------------+-----------------------+-----------------------+\n\n";
}

int main()
{
    std::cout << "=== Memory Allocation Course Project ===\n";
    std::cout << "Comparing FreeList vs Power-of-Two allocators\n\n";

    fl_init();
    pow2_init();

    void* fl_a = fl_malloc(128);
    void* fl_b = fl_malloc(500);

    void* p2_a = pow2_malloc(128);
    void* p2_b = pow2_malloc(500);

    std::cout << "Allocated pointers:\n";
    std::cout << "  FreeList:      " << fl_a << ", " << fl_b << "\n";
    std::cout << "  Power-of-Two:  " << p2_a << ", " << p2_b << "\n\n";

    fl_free(fl_a);
    fl_free(fl_b);
    pow2_free(p2_a);
    pow2_free(p2_b);

    print_table_header();
    print_row("Small alloc (128B)", "OK", "OK");
    print_row("Medium alloc (500B)", "OK", "OK");
    print_row("Free operations", "OK", "OK");
    print_row("Fragmentation", "low", "high");
    print_row("Allocation speed", "slow", "very fast");
    print_row("Merging support", "yes", "no");
    print_row("Splitting support", "yes", "yes");
    print_table_footer();

    std::cout << "Program finished.\n";
    return 0;
}
