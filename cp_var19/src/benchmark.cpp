#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>

#include "freelist_allocator.h"
#include "pow2_allocator.h"
#include "allocator_common.h"

using clock_type = std::chrono::high_resolution_clock;

double now_us()
{
    return std::chrono::duration<double, std::micro>(clock_type::now().time_since_epoch()).count();
}

double percentile(std::vector<double>& v, double p)
{
    if (v.empty()) return 0;
    std::sort(v.begin(), v.end());
    size_t idx = std::min<size_t>(v.size() - 1, size_t(p * v.size()));
    return v[idx];
}

double avg(const std::vector<double>& v)
{
    if (v.empty()) return 0;
    double s = 0;
    for (double x : v) s += x;
    return s / v.size();
}

double variance(const std::vector<double>& v)
{
    if (v.size() < 2) return 0;
    double m = avg(v);
    double s = 0;
    for (double x : v) s += (x - m) * (x - m);
    return s / (v.size() - 1);
}

double jitter(const std::vector<double>& v)
{
    if (v.empty()) return 0;
    return percentile(const_cast<std::vector<double>&>(v), 0.99) - percentile(const_cast<std::vector<double>&>(v), 0.50);
}

struct Metrics
{
    std::vector<double> malloc_times;
    std::vector<double> free_times;

    size_t split_count = 0;
    size_t merge_count = 0;

    size_t internal_frag_sum = 0;
    size_t internal_frag_count = 0;

    size_t current_allocated = 0;
    size_t peak_allocated = 0;

    size_t largest_free_block = 0;
    size_t total_free_memory = 0;
};

size_t internal_frag(size_t requested, size_t allocated)
{
    return (allocated >= requested) ? (allocated - requested) : 0;
}

size_t freelist_largest_block()
{
    size_t maxb = 0;
    Block* cur = fl_get_free_list_head();
    while (cur)
    {
        if (cur->free && cur->size > maxb)
            maxb = cur->size;
        cur = cur->next;
    }
    return maxb;
}

size_t freelist_total_free()
{
    size_t s = 0;
    Block* cur = fl_get_free_list_head();
    while (cur)
    {
        if (cur->free)
            s += cur->size;
        cur = cur->next;
    }
    return s;
}

size_t pow2_largest_block()
{
    size_t maxb = 0;
    for (size_t cls = 0; cls < 32; cls++)
        if (pow2_class_nonempty(cls))
            maxb = (1ULL << cls);
    return maxb;
}

size_t pow2_total_free()
{
    size_t total = 0;
    for (size_t cls = 0; cls < 32; cls++)
        total += pow2_blocks_in_class(cls) * (1ULL << cls);
    return total;
}

// ===============================================================
// RUN TEST
// ===============================================================
void run_test(Metrics& M, void* (*alloc_fn)(size_t), void  (*free_fn)(void*), size_t OPS, bool random_sizes, bool is_freelist)
{
    std::vector<void*> ptrs(OPS);

    for (size_t i = 0; i < OPS; i++)
    {
        size_t req = random_sizes ? (rand() % 2048 + 1) : 128;

        double t0 = now_us();
        void* p = alloc_fn(req);
        double t1 = now_us();

        M.malloc_times.push_back(t1 - t0);
        ptrs[i] = p;

        if (p)
        {
            size_t allocsz = is_freelist ? fl_get_real_block_size(p) : pow2_get_real_block_size(p);

            M.internal_frag_sum += internal_frag(req, allocsz);
            M.internal_frag_count++;

            M.current_allocated += allocsz;
            M.peak_allocated = std::max(M.peak_allocated, M.current_allocated);
        }
    }

    for (size_t i = 0; i < OPS; i++)
    {
        if (!ptrs[i]) continue;

        double t0 = now_us();
        free_fn(ptrs[i]);
        double t1 = now_us();
        M.free_times.push_back(t1 - t0);

        size_t allocsz = is_freelist ? fl_get_real_block_size(ptrs[i]) : pow2_get_real_block_size(ptrs[i]);

        M.current_allocated -= allocsz;
    }
}

void compute_fragmentation(Metrics& M, bool is_freelist)
{
    if (is_freelist)
    {
        M.largest_free_block = freelist_largest_block();
        M.total_free_memory  = freelist_total_free();
    }
    else
    {
        M.largest_free_block = pow2_largest_block();
        M.total_free_memory  = pow2_total_free();
    }
}

void print_metrics(const std::string& title, Metrics& M)
{
    std::cout << "\n========== METRICS: " << title << " ==========\n";

    std::cout << "avg malloc:            " << avg(M.malloc_times) << " us\n";
    std::cout << "avg free:              " << avg(M.free_times) << " us\n";
    std::cout << "p50 malloc:            " << percentile(M.malloc_times, 0.50) << " us\n";
    std::cout << "p95 malloc:            " << percentile(M.malloc_times, 0.95) << " us\n";
    std::cout << "p99 malloc:            " << percentile(M.malloc_times, 0.99) << " us\n";
    std::cout << "variance:              " << variance(M.malloc_times) << "\n";
    std::cout << "jitter (p99-p50):      " << jitter(M.malloc_times) << " us\n";

    std::cout << "splits:                " << M.split_count << "\n";
    std::cout << "merges:                " << M.merge_count << "\n";

    double internal_avg = M.internal_frag_count ? double(M.internal_frag_sum) / M.internal_frag_count : 0;

    std::cout << "avg internal frag:     " << internal_avg << " bytes\n";

    double external = (M.total_free_memory == 0) ? 0 : 100.0 * (1.0 - double(M.largest_free_block) / double(M.total_free_memory));

    std::cout << "largest free block:    " << M.largest_free_block << " bytes\n";
    std::cout << "total free memory:     " << M.total_free_memory << " bytes\n";
    std::cout << "external fragmentation: " << external << " %\n";

    std::cout << "peak allocated:        " << M.peak_allocated << " bytes\n";
}

int main()
{
    const size_t OPS = 200000;

    std::cout << "=== MEMORY ALLOCATORS BENCHMARK ===\n";

    std::cout << "\n[INIT] FreeList...\n";
    fl_init();

    Metrics FLM;
    run_test(FLM, fl_malloc, fl_free, OPS, true, true);

    FLM.split_count = fl_global_split_count;
    FLM.merge_count = fl_global_merge_count;

    compute_fragmentation(FLM, true);
    print_metrics("FreeList", FLM);

    std::cout << "\n[INIT] Power-of-Two...\n";
    pow2_init();

    Metrics P2M;
    run_test(P2M, pow2_malloc, pow2_free, OPS, true, false);

    P2M.split_count = pow2_global_split_count;
    P2M.merge_count = pow2_global_merge_count;

    compute_fragmentation(P2M, false);
    print_metrics("PowerOfTwo", P2M);

    std::cout << "\n=== BENCHMARK FINISHED ===\n";
    return 0;
}
