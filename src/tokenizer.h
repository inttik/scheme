#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <ostream>

#include "int_type.h"

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const;
    
    friend std::ostream& operator<<(std::ostream& out, const SymbolToken& token);
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;

    friend std::ostream& operator<<(std::ostream& out, const QuoteToken& token);
};

struct DotToken {
    bool operator==(const DotToken&) const;

    friend std::ostream& operator<<(std::ostream& out, const DotToken& token);
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    Int value;

    bool operator==(const ConstantToken& other) const;

    friend std::ostream& operator<<(std::ostream& out, const ConstantToken& token);
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

std::ostream& operator<<(std::ostream& out, const Token& token);

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
