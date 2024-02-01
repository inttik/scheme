#include "catch.hpp"
#include "scheme_test.h"

TEST_CASE_METHOD(SchemeTest, "If", "[advanced]") {
    SECTION("Basic") {
        ExpectSyntaxError("(if)");
        ExpectSyntaxError("(if #t)");
        ExpectOutput("(if (< 1 2) 'yay)", "yay");
        ExpectOutput("(if (> 1 2) 'noo)", "()");
        ExpectOutput("(if (= 1 1) 'a 'b )", "a");
        ExpectOutput("(if (= 1 2) 'a 'b )", "b");
        ExpectSyntaxError("(if #t #t #t #t)");
    }
    SECTION("Advanced") {
        Run("(define x 1)");
        Run("(if (= x 2) (set! x 2) (set! x 3))");
        ExpectOutput("x", "3");

        Run("(if (= x 3) (set! x 2) (set! x 3))");
        ExpectOutput("x", "2");
    }
}

TEST_CASE_METHOD(SchemeTest, "Lambdas", "[advanced]") {
    SECTION("Simple") {
        ExpectOutput("((lambda (x) (+ 1 x)) 3)", "4");
        ExpectOutput("((lambda (x) (+ x x)) 0)", "0");
        
        Run("(define hard-func ( lambda (x y) (set! x (+ x x)) (set! y (+ x y)) (+ x y)))");
        ExpectOutput("(hard-func 1 2)", "6");
        ExpectOutput("(hard-func 2 1)", "9");
        
        Run("(define slow-add ( lambda (x y) (if (= x 0 ) y (slow-add (- x 1) (+ y 1)) ) ))");
        ExpectOutput("(slow-add 1000 1000)", "2000");
        ExpectOutput("(slow-add 0 1000000)", "1000000");
    }
    SECTION("Range") {
        Run("(define range ( lambda(x) ( lambda () (set! x (+ x 1)) x )      ))");
        Run("(define r1 (range 10))");
        ExpectOutput("(r1)", "11");
        ExpectOutput("(r1)", "12");
        ExpectOutput("(r1)", "13");
        Run("(define r2 (range 10))");
        ExpectOutput("(r2)", "11");
        ExpectOutput("(r2)", "12");
        ExpectOutput("(r2)", "13");
    }
    SECTION("Lambda sugar") {
        Run("(define (func) 12)");
        ExpectOutput("(func)", "12");

        Run("(define (func x) (+ x 1))");
        ExpectOutput("(func 1)", "2");

        Run("(define (func x y) (+ x y))");
        ExpectOutput("(func 1 1)", "2");

        Run("(define (func x) (if (< x 3) 1 (+ (func (- x 1)) (func (- x 2))) ))");
        ExpectOutput("(func 1)", "1");
        ExpectOutput("(func 2)", "1");
        ExpectOutput("(func 3)", "2");
        ExpectOutput("(func 4)", "3");
        ExpectOutput("(func 5)", "5");
        ExpectOutput("(func 6)", "8");
        ExpectOutput("(func 7)", "13");
        ExpectOutput("(func 8)", "21");
    }
    SECTION("Sharing context") {
        Run("(define (func x) ( cons (lambda () (set! x (+ x 1) ) x) (lambda () (set! x (+ x 2) ) x) ))");
        Run("(define a (func 1))");
        ExpectOutput("((car a))", "2");
        ExpectOutput("((car a))", "3");
        ExpectOutput("((cdr a))", "5");
        ExpectOutput("((cdr a))", "7");
        ExpectOutput("((car a))", "8");
    }
    SECTION("Redefinition") {
        Run("(define plus +)");
        ExpectOutput("(plus 1 2 3 1 2 3)", "12");
        Run("(define (+ x) (if (= x 0) 0 1) )");
        ExpectOutput("(+ 0)", "0");
        ExpectOutput("(+ 1)", "1");
        ExpectOutput("(+ 2)", "1");
        ExpectRuntimeError("(+)");
        ExpectRuntimeError("(+ 1 2)");
    }
}

TEST_CASE_METHOD(SchemeTest, "Pairs", "[advanced]") {
    SECTION("Simple pair") {
        Run("(define x '(1 . 2))");
        ExpectOutput("(car x)", "1");

        Run("(set-car! x 2)");
        ExpectOutput("(car x)", "2");

        Run("(set-cdr! x 3)");
        ExpectOutput("(cdr x)", "3");    
    }
    SECTION("Bad cases") {
        Run("(define x 1)");
        ExpectRuntimeError("(set-car! x 2)");
        ExpectRuntimeError("(set-cdr! x 2)");

        Run("(define x '(1 . 2))");
        ExpectRuntimeError("(set-car! x)");
        ExpectRuntimeError("(set-cdr! x)");
        ExpectRuntimeError("(set-car! x 2 3)");
        ExpectRuntimeError("(set-cdr! x 2 3)");

        Run("(define x '(1 2))");
        Run("(set-car! x 3)");
        Run("(set-cdr! x 4)");
        ExpectOutput("x", "(3 . 4)");
    }
    SECTION("Hard pair") {
        Run("(define x '(1 . 2))");
        Run("(set-car! x x)");
        ExpectOutput("(cdr (car (car (car x))))", "2");

        Run("(define x '(1 . 2))");
        Run("(set-cdr! x x)");
        ExpectOutput("(car (cdr (cdr (cdr x))))", "1");
    }
}

TEST_CASE_METHOD(SchemeTest, "Symbols", "[advanced]") {
    SECTION("Quote works correctly") {
        ExpectNameError("x");

        ExpectOutput("'x", "x");
        ExpectOutput("(quote x)", "x");
    }
    SECTION("Function symbol?") {
        ExpectRuntimeError("(symbol?)");
        Run("(define x 'x)");
        ExpectOutput("(symbol? x)", "#t");
        ExpectOutput("(symbol? 'x)", "#t");
        ExpectOutput("(symbol? 1)", "#f");
        ExpectOutput("(symbol? '1)", "#f");
        ExpectRuntimeError("(symbol? x y)");
    }
    SECTION("Simple symbols") {
        ExpectNameError("(set! x 0)");
        ExpectNameError("x");

        Run("(define x (+ 1 1))");
        ExpectOutput("x", "2");

        Run("(define x (+ 2 2))");
        ExpectOutput("x", "4");

        Run("(set! x (+ 3 3))");
        ExpectOutput("x", "6");
    }
    SECTION("Bad cases") {
        ExpectSyntaxError("(define)");
        ExpectNameError("(define x x)");
        ExpectSyntaxError("(define 1)");
        ExpectSyntaxError("(define 1 2 3)");
        Run("(define x 1)");
        ExpectSyntaxError("(set!)");
        Run("(define x 1)");
        ExpectSyntaxError("(set! x)");
        Run("(define x 1)");
        ExpectSyntaxError("(set! x 2 3)");
    }
    SECTION("Hard logic") {
        Run("(define x 1)");
        Run("(define y x)");
        ExpectOutput("x", "1");
        ExpectOutput("y", "1");
        Run("(define x y)");
        ExpectOutput("x", "1");
        ExpectOutput("y", "1");
        Run("(set! y 2)");
        ExpectOutput("x", "1");
        ExpectOutput("y", "2");
    }
}
