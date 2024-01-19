#include <catch.hpp>

#include <sstream>
#include "object.h"
#include "parser.h"

Object* Parse(const std::string& line) {
    std::istringstream input(line);
    Tokenizer tokenizer(&input);
    Object* ans = Read(&tokenizer);
    
    REQUIRE(tokenizer.IsEnd());
    return ans;
}
