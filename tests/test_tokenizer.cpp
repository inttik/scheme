#include <catch.hpp>

#include <cstdlib>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "error.h"
#include "tokenizer.h"

static std::vector<Token> Parse(const std::string& line) {
    std::istringstream input(line);
    Tokenizer tokenizer(&input);

    std::vector<Token> tokens;
    while (!tokenizer.IsEnd()) {
        tokens.emplace_back(tokenizer.GetToken());
        tokenizer.Next();
    }
    return tokens;
}

static void TestLine(const std::string& line, const std::vector<Token>& correct_order) {
    CAPTURE(line);

    auto order = Parse(line);

    REQUIRE(order.size() == correct_order.size());
    for (size_t i = 0; i < correct_order.size(); ++i) {
        CAPTURE(i);
        REQUIRE(order[i] == correct_order[i]);
    }
}

TEST_CASE("Basic tests", "[tokenizer]") {
    TestLine("", {});
    TestLine("  \t \n\n\t ", {});

    TestLine("42", {ConstantToken{42}});
    TestLine("-4", {ConstantToken{-4}});
    TestLine("+10", {ConstantToken{10}});

    TestLine("(", {BracketToken::OPEN});
    TestLine(")", {BracketToken::CLOSE});

    TestLine("'", {QuoteToken{}});

    TestLine(".", {DotToken{}});

    TestLine("variable", {SymbolToken{"variable"}});

    TestLine("'(+ 4 -5)", {QuoteToken{}, BracketToken::OPEN, SymbolToken{"+"}, ConstantToken{4},
                           ConstantToken{-5}, BracketToken::CLOSE});
}

TEST_CASE("One character symbol token tests", "[tokenizer]") {
    std::string current_input;
    SECTION("Small characters") {
        current_input += GENERATE(range('a', 'z'));
        TestLine(current_input, {SymbolToken{current_input}});
    }
    SECTION("Large characters") {
        current_input += GENERATE(range('A', 'Z'));
        TestLine(current_input, {SymbolToken{current_input}});
    }
    SECTION("Special symbols") {
        std::string allowed_characters = "<=>*/#";
        current_input += GENERATE_COPY(from_range(allowed_characters));
        TestLine(current_input, {SymbolToken{current_input}});
    }
    SECTION("Tricky symbols") {
        TestLine("+", {SymbolToken{"+"}});
        TestLine("-", {SymbolToken{"-"}});
    }
}

TEST_CASE("Multiple character symbol token test", "[tokenizer]") {
    std::string current_input = "A";
    SECTION("Examples") {
        TestLine("We are symbols?",
                 {SymbolToken{"We"}, SymbolToken{"are"}, SymbolToken{"symbols?"}});
        TestLine("+++", {SymbolToken{"+"}, SymbolToken{"+"}, SymbolToken{"+"}});
        TestLine("--+-", {SymbolToken{"-"}, SymbolToken{"-"}, SymbolToken{"+"}, SymbolToken{"-"}});
    }
    SECTION("Special last symbols") {
        std::string unique_characters = "0123456789?!-";
        current_input += GENERATE_COPY(from_range(unique_characters));
        TestLine(current_input, {SymbolToken{current_input}});
    }
}

TEST_CASE("Unknow symbol throw test", "[tokenizer]") {
    std::string current_input;
    std::string bad_start = "?!@$%^&_~`;:,\\|[]{}";
    current_input += GENERATE_COPY(from_range(bad_start));
    CAPTURE(current_input);
    REQUIRE_THROWS_AS(Parse(current_input), SyntaxError);
}

TEST_CASE("Complex symbols", "[tokenizer]") {
    TestLine("aba-caba", {SymbolToken{"aba-caba"}});
}

TEST_CASE("Constant token test", "[tokenizer]") {
    SECTION("Proper ints") {
        Int i = GENERATE(range(-10, 11));
        TestLine(std::to_string(i), {ConstantToken{i}});
    }
    std::string current_input = "+";
    SECTION("Strange ints") {
        Int i = GENERATE(range(0, 11));
        current_input += std::to_string(i);
        TestLine(current_input, {ConstantToken{i}});
    }
}

struct Test {
    std::string input;
    std::vector<Token> correct_output;
};

void AddSymbol(Test& test, const std::string& symbol) {
    test.input += symbol + " ";
    test.correct_output.emplace_back(SymbolToken{symbol});
}
void AddQuote(Test& test) {
    test.input += "'";
    test.correct_output.emplace_back(QuoteToken{});
}
void AddDot(Test& test) {
    test.input += ".";
    test.correct_output.emplace_back(DotToken{});
}
void AddBraket(Test& test, bool is_open) {
    if (is_open) {
        test.input += "(";
        test.correct_output.emplace_back(BracketToken::OPEN);
    } else {
        test.input += ")";
        test.correct_output.emplace_back(BracketToken::CLOSE);
    }
}
void AddConstant(Test& test, Int value, bool add_plus) {
    if (value >= 0 && add_plus) {
        test.input += "+";
    }
    test.input += std::to_string(value) + " ";
    test.correct_output.emplace_back(ConstantToken{value});
}
void ApplyFunction(Test& test, size_t id, std::mt19937& rnd) {
    CAPTURE(id);
    switch (id) {
        case 0:
            AddSymbol(test, "A" + std::to_string(rnd() % 1000));
            break;
        case 1:
            AddQuote(test);
            break;
        case 2:
            AddDot(test);
            break;
        case 3:
            AddBraket(test, rnd() & 1);
            break;
        case 4:
            AddConstant(test, static_cast<Int>(rnd() % 2001) - 1000, rnd() & 1);
            break;
        case 5:
            size_t whitespace_number = rnd() % 10;
            for (size_t i = 0; i < whitespace_number; ++i) {
                std::string whitespaces = " \n\t";
                test.input += whitespaces[rnd() % 3];
            }
    }
}

TEST_CASE("Multiple tokens", "[tokenizer]") {
    std::mt19937 rnd(1231);

    Test current;

    size_t token_id1 = GENERATE(range(0, 6));
    size_t token_id2 = GENERATE(range(0, 6));
    size_t token_id3 = GENERATE(range(0, 6));
    size_t token_id4 = GENERATE(range(0, 6));

    ApplyFunction(current, token_id1, rnd);
    ApplyFunction(current, token_id2, rnd);
    ApplyFunction(current, token_id3, rnd);
    ApplyFunction(current, token_id4, rnd);

    TestLine(current.input, current.correct_output);
}
