#include <catch.hpp>
#include "error.h"
#include "scheme.h"

class SchemeTest {
public:
    void ExpectOutput(const std::string& input, const std::string expect_output) {
        CAPTURE(input);
        const std::string& current_output = interpreter_.Run(input);
        REQUIRE(current_output == expect_output);
    }
    void ExpectSyntaxError(const std::string& input) {
        CAPTURE(input);
        REQUIRE_THROWS_AS(interpreter_.Run(input), SyntaxError);
    }
    void ExpectRuntimeError(const std::string& input) {
        CAPTURE(input);
        REQUIRE_THROWS_AS(interpreter_.Run(input), RuntimeError);
    }
    void ExpectNameError(const std::string& input) {
        CAPTURE(input);
        REQUIRE_THROWS_AS(interpreter_.Run(input), NameError);
    }

private:
    Interpreter interpreter_;
};
