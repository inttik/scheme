#include "catch.hpp"
#include "scheme_test.h"

TEST_CASE_METHOD(SchemeTest, "if", "[advanced]") {
    SECTION("basic") {
        ExpectSyntaxError("(if)");
        ExpectSyntaxError("(if #t)");
        ExpectOutput("(if (< 1 2) 'yay)", "yay");
        ExpectOutput("(if (> 1 2) 'noo)", "()");
        ExpectOutput("(if (= 1 1) 'a 'b )", "a");
        ExpectOutput("(if (= 1 2) 'a 'b )", "b");
        ExpectSyntaxError("(if #t #t #t #t)");
    }
    SECTION("advanced") {
        Run("(define x 1)");
        Run("(if (= x 2) (set! x 2) (set! x 3))");
        ExpectOutput("x", "3");

        Run("(if (= x 3) (set! x 2) (set! x 3))");
        ExpectOutput("x", "2");
    }
}

TEST_CASE_METHOD(SchemeTest, "test lambda") {
}

TEST_CASE_METHOD(SchemeTest, "test pair mut") {
}

TEST_CASE_METHOD(SchemeTest, "test symbol") {
}
