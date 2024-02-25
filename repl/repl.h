#include <iostream>
#include <string_view>

#include <lexer/lexer.h>

namespace repl
{
inline void Start() {
    while (true) {
        std::cout << ">> ";
        std::string line;
        std::getline(std::cin, line);
        if (line.empty()) {
            return;
        }

        auto lexer = Lexer(std::move(line));
        for (auto tok = lexer.NextToken(); tok.type != token::eof; tok = lexer.NextToken()) {
            std::cout << tok.type << std::endl;
        }
    }
}   
} // namespace repl