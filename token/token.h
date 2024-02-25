#ifndef token_token_h
#define token_token_h

#include <iostream>
#include <unordered_map>

struct Token {
    std::string type;
    std::string literal;
};

inline std::string ILLEGAL = "ILLEGAL";

inline std::string my_EOF     = "EOF";

inline std::string IDENT = "IDENT"; // add, foobar, x, y, ...
inline std::string INT   = "INT";   // 1343456

	// Operators
inline std::string ASSIGN   = "=";
inline std::string PLUS     = "+";
inline std::string MINUS    = "-";
inline std::string BANG     = "!";
inline std::string ASTERISK = "*";
inline std::string SLASH    = "/";

inline std::string LT = "<";
inline std::string GT = ">";

inline std::string EQ     = "==";
inline std::string NOT_EQ = "!=";

	// Delimiters
inline std::string COMMA     = ",";
inline std::string SEMICOLON = ";";

inline std::string LPAREN = "(";
inline std::string RPAREN = ")";
inline std::string LBRACE = "{";
inline std::string RBRACE = "}";

	// Keywords
inline std::string FUNCTION = "FUNCTION";
inline std::string LET      = "LET";
inline std::string TRUE     = "TRUE";
inline std::string FALSE    = "FALSE";
inline std::string IF       = "IF";
inline std::string ELSE     = "ELSE";
inline std::string RETURN   = "RETURN";

inline const std::unordered_map<std::string_view, std::string> keywords{
    {"fn",     FUNCTION},
	{"let",    LET},
	{"true",   TRUE},
	{"false",  FALSE},
	{"if",     IF},
	{"else",   ELSE},
	{"return", RETURN},
};

std::string LookupIdent(std::string_view ident)
{
    const auto it = keywords.find(ident);
    if (it == keywords.end()) {
        return IDENT;
    }
    return it->second;
}

#endif // token_token_h