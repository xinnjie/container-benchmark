//
// Created by xinnjie on 2018/11/28.
//
#include <benchmark/benchmark.h>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <iostream>



std::unique_ptr<std::vector<int>> get_random_nums(int n) {
    using namespace std;
    std::srand(std::time(nullptr));
    auto result_p = std::make_unique<std::vector<int>>(n);
    for (int i = 0; i < n; ++i) {
        (*result_p)[i] = std::rand() % n;
    }
    return result_p;
}

static void raw_array_sequential_assign_benchmark(benchmark::State &state) {
    using namespace std;
    int size = state.range(0);
    // volatile: do not optimize following code
    volatile int *v = new int[size];
    for (auto _ : state) {
        for (int i = 0; i < size; ++i) {
            v[i] = i;
        }
    }
    delete[] v;
}

static void raw_array_random_assign_benchmark(benchmark::State &state) {
    using namespace std;
    int size = state.range(0);
    // volatile: do not optimize following code
    volatile int *v = new int[size];
    auto rand_index_p = get_random_nums(size);
    auto rand_values_p = get_random_nums(size);
    for (auto _ : state) {
        for (int i = 0; i < size; ++i) {
            v[(*rand_index_p)[i]] = (*rand_values_p)[i];
        }
    }
    delete[] v;
}



static void std_vector_sequential_assign_benchmark(benchmark::State &state) {
    using namespace std;
    int size = state.range(0);
    vector<int> v(size);

    // 开O3优化时会用到simd指令
    for (auto _ : state) {
        for (int i = 0; i < size; ++i) {
            v[i] = i;
        }
    }
}

static void std_vector_random_assign_benchmark(benchmark::State &state) {
    using namespace std;
    int size = state.range(0);
    vector<int> v(size);

    auto rand_index_p = get_random_nums(size);
    auto rand_values_p = get_random_nums(size);
    for (auto _ : state) {
        for (int i = 0; i < size; ++i) {
            v[(*rand_index_p)[i]] = (*rand_values_p)[i];
        }
    }
}

static void interprocess_vector_sequential_assign_benchmark(benchmark::State &state) {
    using namespace boost::interprocess;
    using shmem_allocator = allocator<int, managed_shared_memory::segment_manager>;
    using vector_int = vector<int, shmem_allocator>;
    struct shm_remove
    {
        shm_remove() { shared_memory_object::remove("benchmark_memory"); }
        ~shm_remove(){ shared_memory_object::remove("benchmark_memory"); }
    } remover;

    managed_shared_memory segment(create_only, "benchmark_memory", 20000000);
    const shmem_allocator alloc_inst (segment.get_segment_manager());
    int size = state.range(0);
    vector_int &v = *segment.construct<vector_int>("vec")(size, alloc_inst);

    // 开优化后inline虽然用到了，但是编译器看不出来设值是连续的样子
    for (auto _ : state) {
        for (int i = 0; i < size; ++i) {
            v[i] = i;
        }
    }
}

static void interprocess_vector_random_assign_benchmark(benchmark::State &state) {
    using namespace boost::interprocess;
    using shmem_allocator = allocator<int, managed_shared_memory::segment_manager>;
    using vector_int = vector<int, shmem_allocator>;
    struct shm_remove
    {
        shm_remove() { shared_memory_object::remove("benchmark_memory"); }
        ~shm_remove(){ shared_memory_object::remove("benchmark_memory"); }
    } remover;

    managed_shared_memory segment(create_only, "benchmark_memory", 20000000);
    const shmem_allocator alloc_inst (segment.get_segment_manager());
    int size = state.range(0);
    vector_int &v = *segment.construct<vector_int>("vec")(size, alloc_inst);

    auto rand_index_p = get_random_nums(size);
    auto rand_values_p = get_random_nums(size);
    for (auto _ : state) {
        for (int i = 0; i < size; ++i) {
            v[(*rand_index_p)[i]] = (*rand_values_p)[i];
        }
    }
}

BENCHMARK(raw_array_sequential_assign_benchmark)->Range(8, 8<<7);
BENCHMARK(raw_array_random_assign_benchmark)->Range(8, 8<<7);
BENCHMARK(std_vector_sequential_assign_benchmark)->Range(8, 8<<7);
BENCHMARK(std_vector_random_assign_benchmark)->Range(8, 8<<7);
BENCHMARK(interprocess_vector_sequential_assign_benchmark)->Range(8, 8<<7);
BENCHMARK(interprocess_vector_random_assign_benchmark)->Range(8, 8<<7);





BENCHMARK_MAIN();
