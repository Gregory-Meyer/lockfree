#include <lf/queue.hpp>

#include <catch2/catch.hpp>

TEST_CASE("lf::Queue::pop with empty Queue") {
    lf::Queue<int> q;

    REQUIRE_FALSE(q.pop());
}

TEST_CASE("lf::Queue::pop with non-empty Queue") {
    lf::Queue<int> q;

    q.push(5);
    q.push(10);
    q.push(15);

    REQUIRE(*q.pop() == 5);
    REQUIRE(*q.pop() == 10);
    REQUIRE(*q.pop() == 15);
    REQUIRE_FALSE(q.pop());
}
