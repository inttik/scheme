#include <catch.hpp>

#include <sstream>
#include "error.h"
#include "object.h"
#include "parser.h"

static Object* Parse(const std::string& line) {
    std::istringstream input(line);
    Tokenizer tokenizer(&input);
    Object* ans = Read(&tokenizer);

    REQUIRE(tokenizer.IsEnd());
    return ans;
}

static void RequireSymbol(Object* object, const std::string& name) {
    REQUIRE(Is<Symbol>(object));
    REQUIRE(As<Symbol>(object)->GetName() == name);
}

static void RequireNumber(Object* object, const Int& value) {
    REQUIRE(Is<Number>(object));
    REQUIRE(As<Number>(object)->GetValue() == value);
}

TEST_CASE("Simple objects", "[parser]") {
    SECTION("Number objects") {
        auto obj = Parse("1");
        RequireNumber(obj, 1);

        obj = Parse("-1");
        RequireNumber(obj, -1);

        obj = Parse("+1");
        RequireNumber(obj, 1);
    }
    SECTION("Symbol objects") {
        auto obj = Parse("+");
        RequireSymbol(obj, "+");

        obj = Parse("aba-caba");
        RequireSymbol(obj, "aba-caba");
    }
    SECTION("Empty list") {
        auto obj = Parse("()");
        REQUIRE(obj == nullptr);
    }
    SECTION("List (- 1 2)") {
        auto obj = Parse("(- 1 2)");
        REQUIRE(Is<Cell>(obj));
        RequireSymbol(As<Cell>(obj)->GetFirst(), "-");
        obj = As<Cell>(obj)->GetSecond();
        REQUIRE(Is<Cell>(obj));
        RequireNumber(As<Cell>(obj)->GetFirst(), 1);
        obj = As<Cell>(obj)->GetSecond();
        REQUIRE(Is<Cell>(obj));
        RequireNumber(As<Cell>(obj)->GetFirst(), 2);
        obj = As<Cell>(obj)->GetSecond();
        REQUIRE(obj == nullptr);
    }
    SECTION("Pair (1 . 2)") {
        auto obj = Parse("(1 . 2)");
        REQUIRE(Is<Cell>(obj));
        RequireNumber(As<Cell>(obj)->GetFirst(), 1);
        RequireNumber(As<Cell>(obj)->GetSecond(), 2);
    }
    SECTION("List (1 2 . 3)") {
        auto obj = Parse("(1 2 . 3)");
        REQUIRE(Is<Cell>(obj));
        RequireNumber(As<Cell>(obj)->GetFirst(), 1);
        obj = As<Cell>(obj)->GetSecond();
        REQUIRE(Is<Cell>(obj));
        RequireNumber(As<Cell>(obj)->GetFirst(), 2);
        RequireNumber(As<Cell>(obj)->GetSecond(), 3);
    }
}

TEST_CASE("Hard correct objects", "[parser]") {
    Parse("(1 . ())");
    Parse("(1 2 . ())");
    Parse("(() . ())");
    Parse("((1 . 2) 3 4 (5 6) . (7 . (8 9)))");
    auto obj = Parse("(())");
    REQUIRE(Is<Cell>(obj));
    REQUIRE(As<Cell>(obj)->GetFirst() == nullptr);
    REQUIRE(As<Cell>(obj)->GetSecond() == nullptr);
}

TEST_CASE("Incorrect objects", "[parser]") {
    REQUIRE_THROWS_AS(Parse(""), SyntaxError);
    REQUIRE_THROWS_AS(Parse("("), SyntaxError);
    REQUIRE_THROWS_AS(Parse(")"), SyntaxError);
    REQUIRE_THROWS_AS(Parse("(()"), SyntaxError);
    REQUIRE_THROWS_AS(Parse("( . )"), SyntaxError);
    REQUIRE_THROWS_AS(Parse("( . 1)"), SyntaxError);
    REQUIRE_THROWS_AS(Parse("( 1 . )"), SyntaxError);
    REQUIRE_THROWS_AS(Parse("( 1 . 2 3)"), SyntaxError);
}

static void RequireExtra(const std::string& line) {
    std::istringstream input(line);
    Tokenizer tokenizer(&input);
    Read(&tokenizer);

    REQUIRE(!tokenizer.IsEnd());
}

TEST_CASE("Parser don't read extra", "[parser]") {
    RequireExtra("1 2");
    RequireExtra("() ()");
    RequireExtra("() aba");
}

TEST_CASE("Quote simple", "[parser]") {
    auto obj = Parse("'1");
    REQUIRE(Is<Cell>(obj));
    RequireSymbol(As<Cell>(obj)->GetFirst(), "quote");
    obj = As<Cell>(obj)->GetSecond();
    REQUIRE(Is<Cell>(obj));
    RequireNumber(As<Cell>(obj)->GetFirst(), 1);
    REQUIRE(As<Cell>(obj)->GetSecond() == nullptr);

    REQUIRE_NOTHROW(Parse("''1"));
}

TEST_CASE("Quote syntax errors", "[parser]") {
    REQUIRE_THROWS_AS(Parse("'"), SyntaxError);
    REQUIRE_THROWS_AS(Parse("''"), SyntaxError);
}
