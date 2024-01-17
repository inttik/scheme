#include <catch.hpp>

int foo(int x);

TEST_CASE("first test") {
    REQUIRE(foo(0) == 1);
    REQUIRE(foo(1) == 2);
}
