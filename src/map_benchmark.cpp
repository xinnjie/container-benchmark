//
// Created by xinnjie on 2018/11/28.
//
#include <benchmark/benchmark.h>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/unordered_map.hpp>
#include <map>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <iostream>

const int LOOKUP_TIMES = 100000;

std::unique_ptr<std::vector<int>> get_random_nums(int n) {
    using namespace std;
    std::srand(std::time(nullptr));
    auto result_p = std::make_unique<std::vector<int>>(n);
    for (int i = 0; i < n; ++i) {
        (*result_p)[i] = std::rand();
    }
    return result_p;
}

template <class Map>
void insert_random_keyvalue(int size, Map& map) {
    std::srand(std::time(nullptr));
    for (int i = 0; i < size;) {
        int random_key = std::rand();
        int random_value = std::rand();
        std::pair<int, int> pair{random_key, random_value};   // 吐槽：interprocess::map没有右值插入
        bool took_place = map.insert(pair).second;
        if (took_place)
            ++i;
    }
}


static void std_map_insert_benchmark(benchmark::State& state) {
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



static void interprocess_map_insert_benchmark(benchmark::State& state) {
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


static void std_map_lookup_benchmark(benchmark::State &state) {
    int size = state.range(0);


    std::map<int, int> m;
    insert_random_keyvalue(size, m);
    auto random_indice_p = get_random_nums(LOOKUP_TIMES);

    for (auto _ : state) {
        for (int i = 0; i < LOOKUP_TIMES; ++i) {
            auto finded = m.find((*random_indice_p)[i]);
        }
    }
}

static void std_hashmap_lookup_benchmark(benchmark::State &state) {
    int size = state.range(0);


    std::unordered_map<int, int> m;
    insert_random_keyvalue(size, m);
    auto random_indice_p = get_random_nums(LOOKUP_TIMES);

    for (auto _ : state) {
        for (int i = 0; i < LOOKUP_TIMES; ++i) {
            auto finded = m.find((*random_indice_p)[i]);
        }
    }
}

static void interprocess_map_lookup_benchmark(benchmark::State &state) {
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
    insert_random_keyvalue(size, m);
    auto random_indice_p = get_random_nums(LOOKUP_TIMES);

    for (auto _ : state) {
        for (int i = 0; i < LOOKUP_TIMES; ++i) {
            auto finded = m.find((*random_indice_p)[i]);
        }
    }
}

static void interprocess_hashmap_lookup_benchmark(benchmark::State &state) {
    using value_type = std::pair<const int, int>;
    using shmem_allocator = boost::interprocess::allocator<value_type , boost::interprocess::managed_shared_memory::segment_manager>;
    using map_int = boost::unordered_map<int, int, boost::hash<int>, std::equal_to<int>, shmem_allocator>;
    struct shm_remove
    {
        shm_remove() { boost::interprocess::shared_memory_object::remove("benchmark_memory"); }
        ~shm_remove(){ boost::interprocess::shared_memory_object::remove("benchmark_memory"); }
    } remover;

    boost::interprocess::managed_shared_memory segment(boost::interprocess::create_only, "benchmark_memory", 800000000);
    const shmem_allocator alloc_inst (segment.get_segment_manager());
    int size = state.range(0);
    map_int &m = *segment.construct<map_int>("map")(3,boost::hash<int>(), std::equal_to<int>(), alloc_inst);
    insert_random_keyvalue(size, m);
    auto random_indice_p = get_random_nums(LOOKUP_TIMES);

    for (auto _ : state) {
        for (int i = 0; i < LOOKUP_TIMES; ++i) {
            auto finded = m.find((*random_indice_p)[i]);
        }
    }
}


BENCHMARK(std_map_insert_benchmark)->Range(8, 8<<20);
BENCHMARK(interprocess_map_insert_benchmark)->Range(8, 8<<20);
BENCHMARK(std_map_lookup_benchmark)->Range(8, 8<<20);
BENCHMARK(std_hashmap_lookup_benchmark)->Range(8, 8<<20);
BENCHMARK(interprocess_map_lookup_benchmark)->Range(8, 8<<20);
BENCHMARK(interprocess_hashmap_lookup_benchmark)->Range(8, 8<<20);



BENCHMARK_MAIN();
