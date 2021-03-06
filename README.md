# container benchmark
实习的公司因为各类原因共享内存用得特别多，为了将数据容器放在共享内存上，也写了很多轮子。刚好看到`Boost:interprocess`，接口是足够现代了，但是不知道性能如何。所以在这里做了一个benchmark。

# dependencies
`boost`
`google/benchmark`

# compile
`cd $benchmark_dir && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release && make`

# Result

## vector
```
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
```
`std::vector`对比`interprocess::vector`在随机赋值时快很多，查看汇编码发现compiler对`std::vector`进行了simd的优化，而对于`interprocess::vector`，compiler就发现不了顺序这个优化点.

对于raw array，我加了`volatile`阻止优化，所以速度比`std::vector`要慢了一些。(如果不对数组加`volatile`修饰符防止优化,那么整段赋值代码都会因为数组本身在后面没有被用到而优化掉)

# map
## 插入
```
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
```
可以发现随着插入数量的增加，速度开始趋同，相比较各类指针偏移操作，本身插入逻辑更耗时间。但是在key个数较小的时候，`interprocess::offset_ptr`的性能损耗是不容忽视的。


## 查询

```
std_map_lookup_benchmark/8                        24 ns         23 ns   29428210
std_map_lookup_benchmark/64                      499 ns        484 ns    1506329
std_map_lookup_benchmark/512                    8235 ns       8208 ns      86711
std_map_lookup_benchmark/4096                 182682 ns     181943 ns       3852
std_map_lookup_benchmark/32768               3731340 ns    3689636 ns        184
std_map_lookup_benchmark/262144             88041652 ns   87878429 ns          7
std_map_lookup_benchmark/2097152          1375593998 ns 1370821000 ns          1
std_map_lookup_benchmark/8388608          8146980286 ns 8032067000 ns          1
std_hashmap_lookup_benchmark/8                   165 ns        160 ns    4420699
std_hashmap_lookup_benchmark/64                 1079 ns       1067 ns     644241
std_hashmap_lookup_benchmark/512                9285 ns       8824 ns      81766
std_hashmap_lookup_benchmark/4096              94218 ns      93241 ns       7391
std_hashmap_lookup_benchmark/32768           1295797 ns    1267359 ns        560
std_hashmap_lookup_benchmark/262144         26021905 ns   25794037 ns         27
std_hashmap_lookup_benchmark/2097152       296494872 ns  290691000 ns          2
std_hashmap_lookup_benchmark/8388608      1378290361 ns 1361531000 ns          1
interprocess_map_lookup_benchmark/8              175 ns        171 ns    4113896
interprocess_map_lookup_benchmark/64            2013 ns       1992 ns     318863
interprocess_map_lookup_benchmark/512          38164 ns      37539 ns      17372
interprocess_map_lookup_benchmark/4096        540232 ns     534830 ns       1295
interprocess_map_lookup_benchmark/32768      9113175 ns    9065047 ns         85
interprocess_map_lookup_benchmark/262144   206415034 ns  200967667 ns          3
interprocess_map_lookup_benchmark/2097152 2863882061 ns 2815927000 ns          1
interprocess_map_lookup_benchmark/8388608 13772586489 ns 13680254000 ns          1
interprocess_hashmap_lookup_benchmark/8              221 ns        198 ns    3189836
interprocess_hashmap_lookup_benchmark/64            1868 ns       1783 ns     413399
interprocess_hashmap_lookup_benchmark/512          30387 ns      16947 ns      40853
interprocess_hashmap_lookup_benchmark/4096        174154 ns     152821 ns       3900
interprocess_hashmap_lookup_benchmark/32768      2893169 ns    2792449 ns        254
interprocess_hashmap_lookup_benchmark/262144    41644076 ns   40875647 ns         17
interprocess_hashmap_lookup_benchmark/2097152  440294224 ns  431269000 ns          2
interprocess_hashmap_lookup_benchmark/8388608 3360580116 ns 2522784000 ns          1
```
以上是在大小为n的容器上进行100,000次查询的耗时
hashmap在容器大时查询速度对比红黑树实现在键-值对数量大于百万后有明显速度优势，同时基于共享内存的hashmap性能与stl的hashmap差异不大，但是红黑树实现的速度差异非常大，基于共享内存的红黑树耗时增长接近现性