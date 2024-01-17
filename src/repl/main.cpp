#include <cstdio>
#include <iostream>
#include <memory>

#include "error.h"
#include "heap.h"
#include "scheme.h"

int main() {
    std::unique_ptr<Interpreter> inter = std::make_unique<Interpreter>();
    while (true) {
        if (std::cin.eof()) {
            break;
        }
        std::string current_command;
        getline(std::cin, current_command);

        try {
            Heap::alloc_count = 0;
            Heap::dealloc_count = 0;
            auto ans = inter->Run(current_command);
            if (!ans.empty()) {
                std::cout << ans << "\n";
            }
            std::cout.flush();
        } catch (SyntaxError& e) {
            std::cout << "Syntax error: " << e.what() << "\n";
        } catch (NameError& e) {
            std::cout << "Name error: " << e.what() << "\n";
        } catch (RuntimeError& e) {
            std::cout << "Runtime error: " << e.what() << "\n";
        }
    }
    return 0;
}
