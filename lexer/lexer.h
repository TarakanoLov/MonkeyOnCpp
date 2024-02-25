#ifndef lexer_lexer_h
#define lexer_lexer_h

#include <string>

#include "../token/token.h"

bool isLetter(uint8_t ch) {
    return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_';
}

bool isDigit(uint8_t ch) {
    return '0' <= ch && ch <= '9';
}

class Lexer
{
public:
    explicit Lexer(std::string input) :
        m_input{std::move(input)}
    {
        this->readChar();
    }

    Token NextToken() {
        Token tok{};

        this->skipWhitespace();

        switch (m_ch)
        {
        case '=':
            if (this->peekChar() == '=') {
                const auto ch = m_ch;
                this->readChar();
                tok = Token(EQ, "==");
            } else {
                tok = Token(ASSIGN, "=");
            }
            break;
        case '+':
            tok = Token(PLUS, "+");
            break;
        case '-':
            tok = Token(PLUS, "-");
            break;
        case '!':
            if (this->peekChar() == '=') {
                this->readChar();
                tok = Token(NOT_EQ, "!=");
            } else {
                tok = Token(BANG, "!");
            }
            break;
        case '/':
            tok = Token(SLASH, "/");
            break;
        case '*':
            tok = Token(ASTERISK, "*");
            break;
        case '<':
            tok = Token(LT, "<");
            break;
        case '>':
            tok = Token(GT, ">");
            break;
        case ';':
            tok = Token(SEMICOLON, ";");
            break;
        case ',':
            tok = Token(COMMA, ",");
            break;
        case '{':
            tok = Token(LBRACE, "{");
            break;
        case '}':
            tok = Token(RBRACE, "}");
            break;
        case '(':
            tok = Token(LPAREN, "(");
            break;
        case ')':
            tok = Token(RPAREN, ")");
            break;
        case 0:
            tok.literal = "";
            tok.type = my_EOF;
            break;
        default:
            if (isLetter(m_ch)) {
                tok.literal = this->readIdentifier();
                tok.type = LookupIdent(tok.literal);
                return tok;
            } else if (isDigit(m_ch)) {
                tok.type = INT;
                tok.literal = this->readNumber();
                return tok;
            } else {
                tok = Token(ILLEGAL, std::to_string(m_ch));
            }
        }

        this->readChar();
	    return tok;
    }

private:
    void readChar() {
        if (this->m_readPosition >= this->m_input.size()) {
            this->m_ch = 0;
        } else {
            this->m_ch = this->m_input[this->m_readPosition];
        }
        this->m_position = this->m_readPosition;
        ++this->m_readPosition;
    }

    void skipWhitespace() {
        while (m_ch == ' ' || m_ch == '\t' || m_ch == '\n' || m_ch == '\r') {
            this->readChar();
        }
    }

    uint8_t peekChar() {
        if (m_readPosition >= m_input.size()) {
            return 0;
        }
        return m_input[m_readPosition];
    }

    std::string readIdentifier() {
        const auto position = m_position;
        while (isLetter(m_ch)) {
            this->readChar();
        }
        return m_input.substr(position, m_position);
    }

    std::string readNumber() {
        const auto position = m_position;
        while (isDigit(m_ch)) {
            this->readChar();
        }
        return m_input.substr(position, m_position);
    }

    std::string m_input;
    int32_t m_position{};
    int32_t m_readPosition{};
    uint8_t m_ch;
};

#endif // lexer_lexer_h