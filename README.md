# container benchmark
实习的公司因为各类原因共享内存用得特别多，为了将数据容器放在共享内存上，也写了很多轮子。刚好看到`Boost:interprocess`，接口是足够现代了，但是不知道性能如何。所以在这里做了一个benchmark。

# dependencies
`boost`
`google/benchmark`

# compile
`cd $benchmark_dir && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release && make`

# Result

## vector
    Run on (4 X 2700 MHz CPU s)
    CPU Caches:
      L1 Data 32K (x2)
      L1 Instruction 32K (x2)
      L2 Unified 262K (x2)
      L3 Unified 3145K (x1)
    Load Average: 4.02, 3.21, 2.97
    --------------------------------------------------------------------------------------------
    Benchmark                                                     Time           CPU Iterations
    --------------------------------------------------------------------------------------------
    raw_array_sequential_assign_benchmark/8                       3 ns          3 ns  217126976
    raw_array_sequential_assign_benchmark/64                     25 ns         23 ns   31862210
    raw_array_sequential_assign_benchmark/512                   172 ns        171 ns    4039448
    raw_array_sequential_assign_benchmark/1024                  338 ns        337 ns    2076603
    raw_array_random_assign_benchmark/8                           5 ns          4 ns  177247049
    raw_array_random_assign_benchmark/64                         31 ns         31 ns   22002961
    raw_array_random_assign_benchmark/512                       270 ns        261 ns    2531069
    raw_array_random_assign_benchmark/1024                      508 ns        507 ns    1387485
    std_vector_sequential_assign_benchmark/8                      2 ns          2 ns  432785345
    std_vector_sequential_assign_benchmark/64                     7 ns          7 ns  104479171
    std_vector_sequential_assign_benchmark/512                   47 ns         47 ns   14883357
    std_vector_sequential_assign_benchmark/1024                  85 ns         85 ns    8173369
    std_vector_random_assign_benchmark/8                          4 ns          4 ns  178199120
    std_vector_random_assign_benchmark/64                        32 ns         31 ns   22105589
    std_vector_random_assign_benchmark/512                      259 ns        258 ns    2680852
    std_vector_random_assign_benchmark/1024                     522 ns        515 ns    1389523
    interprocess_vector_sequential_assign_benchmark/8             8 ns          8 ns   84849513
    interprocess_vector_sequential_assign_benchmark/64           63 ns         63 ns   10913456
    interprocess_vector_sequential_assign_benchmark/512         406 ns        406 ns    1701127
    interprocess_vector_sequential_assign_benchmark/1024        802 ns        800 ns     862611
    interprocess_vector_random_assign_benchmark/8                 9 ns          9 ns   79439836
    interprocess_vector_random_assign_benchmark/64               73 ns         73 ns    9600088
    interprocess_vector_random_assign_benchmark/512             539 ns        536 ns    1227747
    interprocess_vector_random_assign_benchmark/1024           1053 ns       1049 ns     674699
`std::vector`对比`interprocess::vector`在随机赋值时快很多，查看汇编码发现compiler对`std::vector`进行了simd的优化，而对于`interprocess::vector`，compiler就发现不了顺序这个优化点.

对于raw array，我加了`volatile`阻止优化，所以速度比`std::vector`要慢了一些。(如果不对数组加`volatile`修饰符防止优化,那么整段赋值代码都会因为数组本身在后面没有被用到而优化掉)

# map
    Run on (4 X 2700 MHz CPU s)
    CPU Caches:
      L1 Data 32K (x2)
      L1 Instruction 32K (x2)
      L2 Unified 262K (x2)
      L3 Unified 3145K (x1)
    Load Average: 2.70, 2.44, 2.59
    ---------------------------------------------------------------------------------
    Benchmark                                          Time           CPU Iterations
    ---------------------------------------------------------------------------------
    std_map_insert_benchmark/8                        32 ns         32 ns   19554056
    std_map_insert_benchmark/64                      382 ns        381 ns    1784494
    std_map_insert_benchmark/512                   16952 ns      16774 ns      42543
    std_map_insert_benchmark/4096                 343687 ns     332633 ns       2251
    std_map_insert_benchmark/32768               4433556 ns    4423133 ns        143
    std_map_insert_benchmark/262144            130912831 ns  129502800 ns          5
    std_map_insert_benchmark/2097152          2752607513 ns 2679438000 ns          1
    std_map_insert_benchmark/8388608          14061497385 ns 13477410000 ns          1
    interprocess_map_insert_benchmark/8              256 ns        254 ns    2750653
    interprocess_map_insert_benchmark/64            2855 ns       2846 ns     235724
    interprocess_map_insert_benchmark/512          45160 ns      44679 ns      15346
    interprocess_map_insert_benchmark/4096        622551 ns     616071 ns        976
    interprocess_map_insert_benchmark/32768     12957082 ns   12429405 ns         74
    interprocess_map_insert_benchmark/262144   182152925 ns  181168333 ns          3
    interprocess_map_insert_benchmark/2097152 4080912673 ns 3795930000 ns          1
    interprocess_map_insert_benchmark/8388608 17511696908 ns 16895284000 ns          1

可以发现随着插入数量的增加，速度开始趋同，相比较各类指针偏移操作，本身插入逻辑更耗时间。但是在key个数较小的时候，`interprocess::offset_ptr`的性能损耗是不容忽视的。