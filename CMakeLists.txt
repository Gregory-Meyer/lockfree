cmake_minimum_required(VERSION 3.12)

project(lockfree)

find_package(Threads REQUIRED)
find_package(benchmark REQUIRED)

set(BENCH_SOURCES bench/queue.cpp)

add_executable(bench_lockfree bench/main.cpp ${BENCH_SOURCES})
target_link_libraries(bench_lockfree benchmark benchmark_main Threads::Threads)
