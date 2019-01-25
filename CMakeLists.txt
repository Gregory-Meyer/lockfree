cmake_minimum_required(VERSION 3.12)

project(lockfree)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(benchmark REQUIRED)

include_directories(include external/Catch2/single_include)

set(BENCH_SOURCES bench/queue.cpp)

add_executable(bench_lockfree bench/main.cpp ${BENCH_SOURCES})
target_link_libraries(bench_lockfree benchmark benchmark_main Threads::Threads)

set(TEST_SOURCES test/queue.spec.cpp)

add_executable(test_lockfree test/main.spec.cpp ${TEST_SOURCES})
target_link_libraries(test_lockfree Threads::Threads)
