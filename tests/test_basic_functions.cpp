#include "catch.hpp"
#include "scheme_test.h"

TEST_CASE_METHOD(SchemeTest, "Quote", "[basic]") {
    ExpectOutput("(quote 1)", "1");
    ExpectOutput("(quote (1 2 3))", "(1 2 3)");
    ExpectOutput("(quote (((1 (())))))", "(((1 (()))))");
    ExpectOutput("'1", "1");
    ExpectOutput("'(1 2 3)", "(1 2 3)");
    ExpectOutput("'(((1 (()))))", "(((1 (()))))");
}

TEST_CASE_METHOD(SchemeTest, "Integer functions", "[basic]") {
    SECTION("Integer only") {
        ExpectOutput("1", "1");
        ExpectOutput("+1", "1");
        ExpectOutput("-1", "-1");
    }
    SECTION("Function number?") {
        ExpectRuntimeError("(number?)");
        ExpectOutput("(number? 1)", "#t");
        ExpectOutput("(number? -1)", "#t");
        ExpectOutput("(number? 'symbol)", "#f");
        ExpectOutput("(number? (+ 1 2))", "#t");
        ExpectOutput("(number? '(+ 1 2))", "#f");
        ExpectRuntimeError("(number? 1 2)");
    }
    SECTION("Comparison functions") {
        ExpectOutput("(=)", "#t");
        ExpectOutput("(>)", "#t");
        ExpectOutput("(<)", "#t");
        ExpectOutput("(>=)", "#t");
        ExpectOutput("(<=)", "#t");

        ExpectOutput("(= 1)", "#t");
        ExpectOutput("(> 1)", "#t");
        ExpectOutput("(< 1)", "#t");
        ExpectOutput("(>= 1)", "#t");
        ExpectOutput("(<= 1)", "#t");

        ExpectOutput("(= 1 1)", "#t");
        ExpectOutput("(= (+ 1 1) 2)", "#t");
        ExpectOutput("(= 1 2)", "#f");
        ExpectOutput("(= 1 1 1 1)", "#t");
        ExpectOutput("(= 1 1 2 1)", "#f");

        ExpectOutput("(> 2 1)", "#t");
        ExpectOutput("(> 2 (/ 2 2))", "#t");
        ExpectOutput("(> 1 2)", "#f");
        ExpectOutput("(> 1000 100 10 1)", "#t");
        ExpectOutput("(> 5 4 4 3)", "#f");

        ExpectOutput("(< 2 1)", "#f");
        ExpectOutput("(< 2 (/ 2 2))", "#f");
        ExpectOutput("(< 1 2)", "#t");
        ExpectOutput("(< 1 2 4 8)", "#t");
        ExpectOutput("(< 1 2 4 3)", "#f");

        ExpectOutput("(>= 2 1)", "#t");
        ExpectOutput("(>= 2 (/ 2 2))", "#t");
        ExpectOutput("(>= 1 2)", "#f");
        ExpectOutput("(>= 1000 100 10 1)", "#t");
        ExpectOutput("(>= 5 4 4 3)", "#t");
        ExpectOutput("(>= 3 4 4 3)", "#f");

        ExpectOutput("(<= 2 1)", "#f");
        ExpectOutput("(<= (+ 1 1) (/ 2 2))", "#f");
        ExpectOutput("(<= 1 2)", "#t");
        ExpectOutput("(<= 1 2 4 8)", "#t");
        ExpectOutput("(<= 1 2 4 4)", "#t");
        ExpectOutput("(<= 1 2 4 3)", "#f");
    }
    SECTION("Arithmetics functions") {
        ExpectOutput("(+)", "0");
        ExpectOutput("(+ 1)", "1");
        ExpectOutput("(+ 1 2)", "3");
        ExpectOutput("(+ 1 2 3)", "6");

        ExpectOutput("(*)", "1");
        ExpectOutput("(* 1)", "1");
        ExpectOutput("(* 1 2)", "2");
        ExpectOutput("(* 1 2 3)", "6");

        ExpectRuntimeError("(-)");
        ExpectOutput("(- 1)", "1");
        ExpectOutput("(- 1 2)", "-1");
        ExpectOutput("(- 1 2 3)", "-4");

        ExpectRuntimeError("(/)");
        ExpectOutput("(/ 100)", "100");
        ExpectOutput("(/ 100 2)", "50");
        ExpectOutput("(/ 100 2 25)", "2");
        ExpectOutput("(/ 7 4)", "1");
    }
    SECTION("MinMax functions") {
        ExpectRuntimeError("(min)");
        ExpectOutput("(min 1)", "1");
        ExpectOutput("(min 2 3)", "2");
        ExpectOutput("(min 4 3 4)", "3");

        ExpectRuntimeError("(max)");
        ExpectOutput("(max 1)", "1");
        ExpectOutput("(max 2 3)", "3");
        ExpectOutput("(max 4 3 4)", "4");
    }
    SECTION("Function abs") {
        ExpectRuntimeError("(abs)");
        ExpectOutput("(abs 5)", "5");
        ExpectOutput("(abs -5)", "5");
        ExpectRuntimeError("(abs 1 2)");
    }
    SECTION("Integer functions accept only integers") {
        ExpectRuntimeError("(= 1 'a)");
        ExpectRuntimeError("(> 1 'a)");
        ExpectRuntimeError("(< 1 'a)");
        ExpectRuntimeError("(>= 1 'a)");
        ExpectRuntimeError("(<= 1 'a)");

        ExpectRuntimeError("(+ 1 'a)");
        ExpectRuntimeError("(- 1 'a)");
        ExpectRuntimeError("(* 1 'a)");
        ExpectRuntimeError("(/ 1 'a)");

        ExpectRuntimeError("(min 1 'a)");
        ExpectRuntimeError("(max 1 'a)");

        ExpectRuntimeError("(abs 'a)");
    }
}

TEST_CASE_METHOD(SchemeTest, "Boolean functions", "[basic]") {
    SECTION("Boolean only") {
        ExpectOutput("#t", "#t");
        ExpectOutput("#f", "#f");
    }
    SECTION("Function boolean?") {
        ExpectRuntimeError("(boolean?)");
        ExpectOutput("(boolean? #t)", "#t");
        ExpectOutput("(boolean? #f)", "#t");
        ExpectOutput("(boolean? 1)", "#f");
        ExpectOutput("(boolean? (= 1 2))", "#t");
        ExpectOutput("(boolean? '(= 1 2))", "#f");
        ExpectRuntimeError("(boolean? #t #f)");
    }
    SECTION("Function not") {
        ExpectRuntimeError("(not)");
        ExpectOutput("(not #t)", "#f");
        ExpectOutput("(not #f)", "#t");
        ExpectOutput("(not (> 1 2))", "#t");
        ExpectOutput("(not 1)", "#f");
        ExpectOutput("(not 0)", "#f");
        ExpectOutput("(not '())", "#f");
        ExpectRuntimeError("(not #f #f)");
    }
    SECTION("Function and") {
        ExpectOutput("(and)", "#t");
        ExpectOutput("(and 1 2 3)", "3");
        ExpectOutput("(and 1 #f 3)", "#f");
        ExpectOutput("(and #f unknow-symbol)", "#f");
    }
    SECTION("Function or") {
        ExpectOutput("(or)", "#f");
        ExpectOutput("(or 1 2 3)", "1");
        ExpectOutput("(or #f #f (= 1 2))", "#f");
        ExpectOutput("(or #t unknow-symbol)", "#t");
    }
}

TEST_CASE_METHOD(SchemeTest, "List functions", "[basic]") {
    SECTION("List only") {
        ExpectRuntimeError("()");
        ExpectRuntimeError("(1)");
        ExpectRuntimeError("(1 2)");
        ExpectRuntimeError("(1 . 2)");
    }
    SECTION("List Syntax") {
        ExpectOutput("'(1 2)", "(1 2)");
        ExpectOutput("'(1 . 2)", "(1 . 2)");
        ExpectOutput("'(1 2 . 3)", "(1 2 . 3)");
        ExpectOutput("'(1 . (2 . ()))", "(1 2)");
        ExpectOutput("'(1 2 . ())", "(1 2)");
    }
    SECTION("Function pair?") {
        ExpectRuntimeError("(pair?)");
        ExpectOutput("(pair? '(1))", "#t");
        ExpectOutput("(pair? '(1 . 2))", "#t");
        ExpectOutput("(pair? '(1 2))", "#t");
        ExpectOutput("(pair? '(1 2 3))", "#t");
        ExpectOutput("(pair? '(()))", "#t");
        ExpectOutput("(pair? 1)", "#f");
        ExpectRuntimeError("(pair? 1 2)");
    }
    SECTION("Function null?") {
        ExpectRuntimeError("(null?)");
        ExpectOutput("(null? '())", "#t");
        ExpectOutput("(null? '(()))", "#f");
        ExpectRuntimeError("(null? 1 2)");
    }
    SECTION("Function list?") {
        ExpectRuntimeError("(list?)");
        ExpectOutput("(list? '())", "#t");
        ExpectOutput("(list? '(1 2))", "#t");
        ExpectOutput("(list? '(1 . 2))", "#f");
        ExpectOutput("(list? 1)", "#f");
        ExpectRuntimeError("(list? 1 2)");
    }
    SECTION("Function cons") {
        ExpectRuntimeError("(cons 1)");
        ExpectOutput("(cons 1 2)", "(1 . 2)");
        ExpectOutput("(cons '(1 2) 2)", "((1 2) . 2)");
        ExpectRuntimeError("(cons 1 2 3)");
    }
    SECTION("Function car") {
        ExpectRuntimeError("(car)");
        ExpectRuntimeError("(car 1)");
        ExpectRuntimeError("(car '())");
        ExpectOutput("(car '(1))", "1");
        ExpectOutput("(car '(1 . 2))", "1");
        ExpectOutput("(car '(1  2))", "1");
        ExpectOutput("(car '(1  2 . 3))", "1");
        ExpectRuntimeError("(car '(1 . 2) '(2 . 3))");
    }
    SECTION("Function cdr") {
        ExpectRuntimeError("(cdr)");
        ExpectRuntimeError("(cdr 1)");
        ExpectRuntimeError("(cdr '())");
        ExpectOutput("(cdr '(1))", "()");
        ExpectOutput("(cdr '(1 . 2))", "2");
        ExpectOutput("(cdr '(1  2))", "(2)");
        ExpectOutput("(cdr '(1  2 . 3))", "(2 . 3)");
        ExpectRuntimeError("(cdr '(1) '(2))");
    }
    SECTION("Function list") {
        ExpectOutput("(list)", "()");
        ExpectOutput("(list 1 2)", "(1 2)");
        ExpectOutput("(list '(1) '(2))", "((1) (2))");
    }
    SECTION("Function list-ref") {
        ExpectRuntimeError("(list-ref)");
        ExpectRuntimeError("(list-ref 1)");
        ExpectRuntimeError("(list-ref 1 2)");
        ExpectRuntimeError("(list-ref '(1) 0 0)");
        ExpectOutput("(list-ref '(1 2 3 4) 0)", "1");
        ExpectOutput("(list-ref '(1 2 3 4) 1)", "2");
        ExpectOutput("(list-ref '(1 2 3 4) 2)", "3");
        ExpectOutput("(list-ref '(1 2 3 4) 3)", "4");
        ExpectRuntimeError("(list-ref '(1 2 3 4) 4)");
        ExpectOutput("(list-ref '(1 . 2) 0)", "1");
        ExpectOutput("(list-ref '(1 . 2) 1)", "2");
        ExpectRuntimeError("(list-ref '(1 . 2) 2)");
    }
    SECTION("Function list-tail") {
        ExpectRuntimeError("(list-tail)");
        ExpectRuntimeError("(list-tail 1)");
        ExpectRuntimeError("(list-tail 1 2)");
        ExpectRuntimeError("(list-tail '(1) 0 0)");
        ExpectOutput("(list-tail '(1 2 3 4) 0)", "(1 2 3 4)");
        ExpectOutput("(list-tail '(1 2 3 4) 1)", "(2 3 4)");
        ExpectOutput("(list-tail '(1 2 3 4) 2)", "(3 4)");
        ExpectOutput("(list-tail '(1 2 3 4) 3)", "(4)");
        ExpectOutput("(list-tail '(1 2 3 4) 4)", "()");
        ExpectRuntimeError("(list-tail '(1 2 3 4) 5)");
        ExpectOutput("(list-tail '(1 . 2) 0)", "(1 . 2)");
        ExpectOutput("(list-tail '(1 . 2) 1)", "2");
        ExpectRuntimeError("(list-tail '(1 . 2) 2)");
    }
}
