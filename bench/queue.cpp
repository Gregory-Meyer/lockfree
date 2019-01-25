#include <lf/queue.hpp>

#include <benchmark/benchmark.h>

static void bm_Queue_push(benchmark::State &state) {
    for (auto _ : state) {

    }
}
BENCHMARK(bm_Queue_push)->UseManualTime();
