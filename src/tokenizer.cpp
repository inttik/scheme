#include "tokenizer.h"
#include <cctype>
#include "error.h"
#include "int_type.h"

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
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
    if ('a' <= ch && ch <= 'z') {
        return true;
    }
    if ('A' <= ch && ch <= 'Z') {
        return true;
    }
    std::string correct = "<=>*/#";
    if (correct.find(ch) != std::string::npos) {
        return true;
    }
    return false;
}

static bool IsCorrectInsideSymbol(char ch) {
    if (isdigit(ch)) {
        return true;
    }
    std::string correct = "?!-";
    if (correct.find(ch) != std::string::npos) {
        return true;
    }
    return IsCorrectBeginSymbol(ch);
}

void Tokenizer::ReadToken() {
    while (!in_->eof() && iswspace(Peek(in_))) {
        Get(in_);
    }
    if (in_->eof()) {
        is_end_ = true;
        return;
    }
    bool first = true;
    bool digit_now = false;
    int digit_sign = 1;
    Int value = 0;
    bool symbol_now = false;
    std::string symbol;
    while (true) {
        if (first) {
            first = false;
            char current = Get(in_);
            if (current == '(') {
                last_token_ = BracketToken::OPEN;
                return;
            } else if (current == ')') {
                last_token_ = BracketToken::CLOSE;
                return;
            } else if (current == '\'') {
                last_token_ = QuoteToken();
                return;
            } else if (current == '.') {
                last_token_ = DotToken();
                return;
            } else if (isdigit(current)) {
                digit_now = true;
                digit_sign = 1;
                value += current - '0';
                continue;
            } else if (IsCorrectBeginSymbol(current)) {
                symbol_now = true;
                symbol += current;
                continue;
            } else if (current == '+' || current == '-') {
                if (in_->eof() || !isdigit(Peek(in_))) {
                    symbol = "";
                    symbol += current;
                    last_token_ = SymbolToken{symbol};
                    return;
                }
                digit_now = true;
                digit_sign = 1;
                if (current == '-') {
                    digit_sign = -1;
                }
                continue;
            } else {
                std::string message = "unexpected character '";
                message.push_back(current);
                message.push_back('\'');
                throw SyntaxError(message);
            }
            continue;
        }
        if (digit_now) {
            if (in_->eof()) {
                last_token_ = ConstantToken{value};
                return;
            }
            char will_next = Peek(in_);
            if (isdigit(will_next)) {
                Get(in_);
                if (value > kIntMax / 10 || value < kIntMin / 10) {
                    throw SyntaxError("number can't be presented as Int");
                }
                value *= 10;
                if (digit_sign == 1) {
                    int to_add = will_next - '0';
                    if (value > kIntMax - to_add) {
                        throw SyntaxError("number can't be presented as Int");
                    }
                    value += to_add;
                } else {
                    int to_add = will_next - '0';
                    if (value < kIntMin + to_add) {
                        throw SyntaxError("number can't be presented as Int");
                    }
                    value -= to_add;
                }
                continue;
            }
            last_token_ = ConstantToken{value};
            return;
        }
        if (symbol_now) {
            if (in_->eof()) {
                last_token_ = SymbolToken{symbol};
                return;
            }
            char will_next = Peek(in_);
            if (IsCorrectInsideSymbol(will_next)) {
                Get(in_);
                symbol += will_next;
                continue;
            }
            last_token_ = SymbolToken{symbol};
            return;
        }
    }
}
