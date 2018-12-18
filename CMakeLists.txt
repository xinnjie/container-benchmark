cmake_minimum_required(VERSION 3.12)
project(container_benchmark)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g")

find_library(boost REQUIRED COMPONENTS
             interprocess date_time)
add_subdirectory(third-party/benchmark EXCLUDE_FROM_ALL)

add_executable(map_benchmark src/map_benchmark.cpp)
target_link_libraries(map_benchmark ${Boost_LIBRARIES}
                                    benchmark)


add_executable(vector_benchmark src/vector_benchmark.cpp)
target_link_libraries(vector_benchmark ${Boost_LIBRARIES}
                                        benchmark)

