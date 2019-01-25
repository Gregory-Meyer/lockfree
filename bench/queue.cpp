#include <lf/queue.hpp>

#include <benchmark/benchmark.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

template <typename T>
class LockingQueue {
public:
    void push(const T &value) {
        emplace(value);
    }

    void push(T &&value) {
        emplace(std::move(value));
    }

    template <typename ...Ts, std::enable_if_t<std::is_constructible_v<T, Ts...>, int> = 0>
    void emplace(Ts &&...ts) {
        const std::scoped_lock lck{ mtx_ };

        queue_.emplace(std::forward<Ts>(ts)...);
    }

    template <typename U, typename ...Ts, std::enable_if_t<
        std::is_constructible_v<T, std::initializer_list<U>&, Ts...>,
        int
    > = 0>
    void emplace(std::initializer_list<U> list, Ts &&...ts) {
        const std::scoped_lock lck{ mtx_ };

        queue_.emplace(list, std::forward<Ts>(ts)...);
    }

    std::optional<T> pop() {
        const std::scoped_lock lck{ mtx_ };

        if (queue_.empty()) {
            return std::nullopt;
        }

        const std::optional<T> value{ std::move(queue_.front()) };
        queue_.pop();

        return value;
    }

private:
    std::queue<T> queue_;
    std::mutex mtx_;
};

static void bm_Queue_push(benchmark::State &state) {
    using Vec = std::vector<std::thread>;
    using Size = Vec::size_type;

    const auto num_readers{ static_cast<Size>(state.range(0)) };
    const auto num_writers{ static_cast<Size>(state.range(1)) };

    const auto stop_flag_ptr{ std::make_shared<std::atomic<bool>>(false) };
    const auto queue_ptr{ std::make_shared<lf::Queue<int>>() };

    Vec threads;
    threads.reserve(num_writers + num_readers);

    for (Size i = 0; i < num_writers; ++i) {
        threads.emplace_back([queue_ptr, stop_flag_ptr] {
            while (!*stop_flag_ptr) {
                queue_ptr->push(0);
            }
        });
    }

    for (Size i = 0; i < num_readers; ++i) {
        threads.emplace_back([queue_ptr, stop_flag_ptr] {
            while (!*stop_flag_ptr) {
                queue_ptr->pop();
            }
        });
    }

    for (auto _ : state) {
        queue_ptr->push(5);
    }

    *stop_flag_ptr = true;

    for (auto &thread : threads) {
        thread.join();
    }
}
BENCHMARK(bm_Queue_push)->Ranges({ {1,  512 }, { 1, 512 } });
