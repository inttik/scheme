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
    auto first = Read(tokenizer);
    current_token = SafeGet(tokenizer);
    if ([[maybe_unused]] DotToken* current = std::get_if<DotToken>(&current_token)) {
        tokenizer->Next();
        auto second = Read(tokenizer);
        current_token = SafeGet(tokenizer);
        if (BracketToken* current = std::get_if<BracketToken>(&current_token)) {
            if (*current == BracketToken::CLOSE) {
                tokenizer->Next();
                return heap->Make<Cell>(first, second);
            }
        }
        throw SyntaxError("List haven't ended with close bracket");
    }
    auto second = ReadList(tokenizer);
    return heap->Make<Cell>(first, second);
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
