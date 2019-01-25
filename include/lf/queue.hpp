#ifndef LF_QUEUE_HPP
#define LF_QUEUE_HPP

#include <cassert>
#include <cstddef>
#include <atomic>
#include <initializer_list>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace lf {
namespace queue {

template <typename T>
struct Node {
    template <typename ...Ts, std::enable_if_t<std::is_constructible_v<T, Ts...>, int> = 0>
    explicit Node(Ts &&...ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>);

    template <typename U, typename ...Ts, std::enable_if_t<
        std::is_constructible_v<T, std::initializer_list<U>&, Ts...>,
        int
    > = 0>
    explicit Node(std::initializer_list<U> list, Ts &&...ts)
    noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Ts...>);

    explicit Node(std::monostate) noexcept;

    std::shared_ptr<Node> next;
    std::variant<std::monostate, T> value;
};

} // namespace queue

template <typename T>
class Queue {
public:
    static_assert(std::is_copy_constructible_v<T>);

    void push(const T &value);

    void push(T &&value);

    template <typename ...Ts, std::enable_if_t<std::is_constructible_v<T, Ts...>, int> = 0>
    void emplace(Ts &&...ts);

    template <typename U, typename ...Ts, std::enable_if_t<
        std::is_constructible_v<T, std::initializer_list<U>&, Ts...>,
        int
    > = 0>
    void emplace(std::initializer_list<U> list, Ts &&...ts);

    std::optional<T> pop() noexcept(std::is_nothrow_copy_constructible_v<T>);

private:
    using Node = queue::Node<T>;
    using NodePtr = std::shared_ptr<Node>;

    void push(std::unique_ptr<Node> node_ptr) noexcept;

    NodePtr head_{ std::make_shared<Node>(std::monostate{ }) };
    NodePtr tail_{ head_ };
};

template <typename T>
void Queue<T>::push(const T &value) {
    push(std::make_unique<queue::Node<T>>(value));
}

template <typename T>
void Queue<T>::push(T &&value) {
    push(std::make_unique<queue::Node<T>>(std::move(value)));
}

template <typename T>
template <typename ...Ts, std::enable_if_t<std::is_constructible_v<T, Ts...>, int>>
void Queue<T>::emplace(Ts &&...ts) {
    push(std::make_unique<queue::Node<T>>(std::forward<Ts>(ts)...));
}

template <typename T>
template <typename U, typename ...Ts, std::enable_if_t<
    std::is_constructible_v<T, std::initializer_list<U>&, Ts...>,
    int
>>
void Queue<T>::emplace(std::initializer_list<U> list, Ts &&...ts) {
    push(std::make_unique<queue::Node<T>>(list, std::forward<Ts>(ts)...));
}

template <typename T>
std::optional<T> Queue<T>::pop() noexcept(std::is_nothrow_copy_constructible_v<T>) {
    while (true) {
        NodePtr head{ std::atomic_load(&head_) };
        NodePtr tail{ std::atomic_load(&tail_) };
        const NodePtr next{ std::atomic_load(&head->next) };

        if (head != std::atomic_load(&head_)) {
            continue;
        }

        if (head == tail) {
            if (!next) {
                return std::nullopt;
            }

            std::atomic_compare_exchange_weak(&tail_, &tail, next);
        } else {
            const std::optional<T> value{ std::get<T>(next->value) };

            if (std::atomic_compare_exchange_weak(&head_, &head, next)) {
                return value;
            }
        }
    }
}

template <typename T>
void Queue<T>::push(std::unique_ptr<Node> node_ptr) noexcept {
    assert(node_ptr);

    const NodePtr new_tail{ std::move(node_ptr) };
    NodePtr tail;

    while (true) {
        tail = std::atomic_load(&tail_);
        NodePtr next{ std::atomic_load(&tail->next) };

        if (tail != std::atomic_load(&tail_)) {
            continue;
        }

        if (!next) {
            if (std::atomic_compare_exchange_weak(&tail->next, &next, new_tail)) {
                break;
            }
        } else {
            std::atomic_compare_exchange_weak(&tail_, &tail, next);
        }
    }

    std::atomic_compare_exchange_weak(&tail_, &tail, new_tail);
}

namespace queue {

template <typename T>
template <typename ...Ts, std::enable_if_t<std::is_constructible_v<T, Ts...>, int>>
Node<T>::Node(Ts &&...ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
: value(std::in_place_type<T>, std::forward<Ts>(ts)...) { }

template <typename T>
template <typename U, typename ...Ts, std::enable_if_t<
    std::is_constructible_v<T, std::initializer_list<U>&, Ts...>,
    int
>>
Node<T>::Node(std::initializer_list<U> list, Ts &&...ts)
noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Ts...>)
: value(std::in_place_type<T>, list, std::forward<Ts>(ts)...) { }

template <typename T>
Node<T>::Node(std::monostate) noexcept : value(std::in_place_type<std::monostate>) { }

} // namespace queue
} // namespace lf

#endif
