#ifndef lexer_lexer_h
#define lexer_lexer_h

#include <string>

#include <token/token.h>

namespace lexer
{

bool isLetter(uint8_t ch);

bool isDigit(uint8_t ch);

class Lexer
{
public:
    explicit Lexer(std::string input);

    token::Token NextToken();

private:
    void readChar();

    void skipWhitespace();

    uint8_t peekChar();

    std::string readIdentifier();

    std::string readNumber();

    std::string m_input;
    int32_t m_position{};
    int32_t m_readPosition{};
    uint8_t m_ch;
};

}

#endif // lexer_lexer_h