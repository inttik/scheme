#include "tokenizer.h"
#include <cctype>
#include <cstdlib>
#include <ostream>
#include <sstream>
#include <string>
#include <regex>
#include <variant>
#include "error.h"
#include "int_type.h"

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}

std::ostream& operator<<(std::ostream& out, const SymbolToken& token) {
    out << "[Symbol token {" << token.name << "}]";
    return out;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

std::ostream& operator<<(std::ostream& out, const QuoteToken& token) {
    out << "[Quote token]";
    return out;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

std::ostream& operator<<(std::ostream& out, const DotToken& token) {
    out << "[Dot token]";
    return out;
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

std::ostream& operator<<(std::ostream& out, const ConstantToken& token) {
    out << "[Constant token {" << token.value << "}]";
    return out;
}

std::ostream& operator<<(std::ostream& out, const Token& token) {
    {
        auto* value = std::get_if<0>(&token);
        if (value != nullptr) {
            out << *value;
        }
    }
    {
        auto* value = std::get_if<1>(&token);
        if (value != nullptr) {
            out << *value;
        }
    }
    {
        auto* value = std::get_if<2>(&token);
        if (value != nullptr) {
            out << *value;
        }
    }
    {
        auto* value = std::get_if<3>(&token);
        if (value != nullptr) {
            out << *value;
        }
    }
    {
        auto* value = std::get_if<4>(&token);
        if (value != nullptr) {
            out << *value;
        }
    }
    return out;
}

Tokenizer::Tokenizer(std::istream* in) {
    in_ = in;
    is_end_ = false;
    last_token_ = {};
    ReadToken();
}
bool Tokenizer::IsEnd() {
    return is_end_;
}
Token Tokenizer::GetToken() {
    return last_token_;
}
void Tokenizer::Next() {
    ReadToken();
}

static char Get(std::istream* in) {
    if (!in->good()) {
        throw SyntaxError("cannot read token due bad symbol");
    }
    char ch = in->get();
    return ch;
}

static char Peek(std::istream* in) {
    if (!in->good()) {
        throw SyntaxError("cannot read token due bad symbol");
    }
    char ch = in->peek();
    return ch;
}

static bool IsCorrectBeginSymbol(char ch) {
    static const std::regex kBeginRegex("[a-zA-Z<=>*/#]+");
    std::string tempo;
    tempo += ch;
    return std::regex_match(tempo, kBeginRegex);
}

static bool IsCorrectInsideSymbol(char ch) {
    static const std::regex kInsideRegex("[a-zA-Z<=>*/#0-9?!-]+");
    std::string tempo;
    tempo += ch;
    return std::regex_match(tempo, kInsideRegex);
}

static Int ToInt(const std::string& value) {
    static std::stringstream ss;
    ss.clear();
    ss << value;
    Int ans;
    ss >> ans;
    if (!ss.eof()) {
        throw SyntaxError("number can't be presented as Int");
    }
    return ans;
}

void Tokenizer::ReadToken() {
    while (!in_->eof() && iswspace(Peek(in_))) {
        Get(in_);
    }
    if (in_->eof()) {
        is_end_ = true;
        return;
    }

    char current = Peek(in_);
    if (current == '(') {
        Get(in_);
        last_token_ = BracketToken::OPEN;
        return;
    }
    if (current == ')') {
        Get(in_);
        last_token_ = BracketToken::CLOSE;
        return;
    }
    if (current == '\'') {
        Get(in_);
        last_token_ = QuoteToken();
        return;
    }
    if (current == '.') {
        Get(in_);
        last_token_ = DotToken();
        return;
    }

    current = Get(in_);
    if (current == '+' || current == '-') {
        if (in_->eof()) {
            std::string symbol;
            symbol += current;
            last_token_ = SymbolToken{symbol};
            return;
        }
        char next = Peek(in_);
        if (isdigit(next)) {
            std::string value;
            value += current;
            while (!in_->eof() && isdigit(Peek(in_))) {
                value += Get(in_);
            }
            last_token_ = ConstantToken{ToInt(value)};
            return;
        }
        std::string symbol;
        symbol += current;
        last_token_ = SymbolToken{symbol};
        return;
    }
    if (isdigit(current)) {
        std::string value;
        value += current;
        while (!in_->eof() && isdigit(Peek(in_))) {
            value += Get(in_);
        }
        last_token_ = ConstantToken{ToInt(value)};
        return;
    }
    if (IsCorrectBeginSymbol(current)) {
        std::string symbol;
        symbol += current;
        while (!in_->eof() && IsCorrectInsideSymbol(Peek(in_))) {
            symbol += Get(in_);
        }
        last_token_ = SymbolToken{symbol};
        return;
    }

    std::string message = "unexpected character '";
    message += current;
    message += '\'';
    throw SyntaxError(message);
}
