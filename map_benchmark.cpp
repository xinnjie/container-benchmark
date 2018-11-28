//
// Created by xinnjie on 2018/11/28.
//
#include <benchmark/benchmark.h>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <map>
#include <cstdlib>
#include <ctime>
#include <iostream>



std::unique_ptr<std::vector<int>> get_random_nums(int n) {
    using namespace std;
    std::srand(std::time(nullptr));
    auto result_p = std::make_unique<std::vector<int>>(n);
    for (int i = 0; i < n; ++i) {
        (*result_p)[i] = std::rand();
    }
    return result_p;
}


static void std_map_insert_benchmark(benchmark::State &state) {
    using namespace std;
    int size = state.range(0);
    map<int, int> m;

    auto rand_index_p = get_random_nums(size);
    auto rand_values_p = get_random_nums(size);
    for (auto _ : state) {
        for (int i = 0; i < size; ++i) {
            m[(*rand_index_p)[i]] = (*rand_values_p)[i];
        }
    }
}



static void interprocess_map_insert_benchmark(benchmark::State &state) {
    using namespace boost::interprocess;
    using value_type = std::pair<const int, int>;
    using shmem_allocator = allocator<value_type , managed_shared_memory::segment_manager>;
    using map_int = map<int, int, std::less<int>, shmem_allocator>;
    struct shm_remove
    {
        shm_remove() { shared_memory_object::remove("benchmark_memory"); }
        ~shm_remove(){ shared_memory_object::remove("benchmark_memory"); }
    } remover;

    managed_shared_memory segment(create_only, "benchmark_memory", 800000000);
    const shmem_allocator alloc_inst (segment.get_segment_manager());
    int size = state.range(0);
    map_int &m = *segment.construct<map_int>("map")(std::less<int>(), alloc_inst);

    auto rand_index_p = get_random_nums(size);
    auto rand_values_p = get_random_nums(size);
    for (auto _ : state) {
        for (int i = 0; i < size; ++i) {
            m[(*rand_index_p)[i]] = (*rand_values_p)[i];
        }
    }
}


BENCHMARK(std_map_insert_benchmark)->Range(8, 8<<20);
BENCHMARK(interprocess_map_insert_benchmark)->Range(8, 8<<20);


BENCHMARK_MAIN();
