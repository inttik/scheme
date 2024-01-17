#pragma once

#include <variant>
#include <optional>
#include <istream>

#include "int_type.h"

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    Int value;

    bool operator==(const ConstantToken& other) const;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    void Next();

    bool IsEnd();
    Token GetToken();

private:
    void ReadToken();

    bool is_end_;
    std::istream* in_;
    Token last_token_;
};
