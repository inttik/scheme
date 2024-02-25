#include "parser.h"

#include "error.h"
#include "heap.h"
#include "object.h"
#include "tokenizer.h"

static Token SafeGet(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("Unexpected end of line");
    }
    return tokenizer->GetToken();
}

static Object* ReadList(Tokenizer* tokenizer) {
    static auto heap = GetHeap();

    auto current_token = SafeGet(tokenizer);
    if (BracketToken* current = std::get_if<BracketToken>(&current_token)) {
        if (*current == BracketToken::CLOSE) {
            tokenizer->Next();
            return nullptr;
        }
    }

    auto ans = heap->Make<Cell>(nullptr, nullptr);
    ans->SetFirst(Read(tokenizer));

    auto pointer = ans;

    while (true) {
        auto current_token = SafeGet(tokenizer);
        if (BracketToken* current = std::get_if<BracketToken>(&current_token)) {
            if (*current == BracketToken::CLOSE) {
                tokenizer->Next();
                break;
            }
        }
        if ([[maybe_unused]] DotToken* current = std::get_if<DotToken>(&current_token)) {
            tokenizer->Next();
            pointer->SetSecond(Read(tokenizer));
            current_token = SafeGet(tokenizer);
            if (BracketToken* current = std::get_if<BracketToken>(&current_token)) {
                if (*current == BracketToken::CLOSE) {
                    tokenizer->Next();
                    break;
                }
            }
            throw SyntaxError("Improper list haven't ended with close bracket");
        }

        pointer->SetSecond(heap->Make<Cell>(Read(tokenizer), nullptr));
        pointer = As<Cell>(pointer->GetSecond());
    }

    return ans;
}

Object* Read(Tokenizer* tokenizer) {
    static auto heap = GetHeap();
    auto current_token = SafeGet(tokenizer);
    if (ConstantToken* current = std::get_if<ConstantToken>(&current_token)) {
        tokenizer->Next();
        return heap->Make<Number>(current->value);
    }
    if (SymbolToken* current = std::get_if<SymbolToken>(&current_token)) {
        tokenizer->Next();
        return heap->Make<Symbol>(current->name);
    }
    if ([[maybe_unused]] QuoteToken* current = std::get_if<QuoteToken>(&current_token)) {
        tokenizer->Next();
        auto first = heap->Make<Symbol>("quote");
        auto arg = Read(tokenizer);
        auto second = heap->Make<Cell>(arg, nullptr);
        return heap->Make<Cell>(first, second);
    }
    if ([[maybe_unused]] DotToken* current = std::get_if<DotToken>(&current_token)) {
        throw SyntaxError("Unexpected dot token");
    }
    if (BracketToken* current = std::get_if<BracketToken>(&current_token)) {
        if (*current == BracketToken::CLOSE) {
            throw SyntaxError("Unexpected close bracket");
        }
        tokenizer->Next();
        return ReadList(tokenizer);
    }
    throw SyntaxError("Unexpected token");
}
