#ifndef parser_parser_h
#define parser_parser_h

#include <fmt/core.h>

#include <ast/ast.h>
#include <lexer/lexer.h>
#include <token/token.h>

struct Parser {
    Parser(lexer::Lexer lexer) : l(std::move(lexer)) {
       this->nextToken();
       this->nextToken();
    }

    void nextToken() {
        this->curToken = this->peekToken;
        this->peekToken = this->l.NextToken();
    }

    ast::Program ParseProgram() {
        ast::Program program;

        while (this->curToken.type != token::eof) {
            const auto stmt = this->parseStatement();
            program.statements.push_back(stmt);
            this->nextToken();
        }
        return program;
    }

    const std::vector<std::string>& Errors() {
        return this->errors;
    }

    std::shared_ptr<ast::Statement> parseStatement() {
        if (this->curToken.type == token::LET) {
            return this->parseLetStatement();
        }
        return {};
    }

    std::shared_ptr<ast::Statement> parseLetStatement() {
        auto stmt = std::make_shared<ast::LetStatement>();
        stmt->token = this->curToken;

        if (!this->expectPeek(token::IDENT)) {
            return {};
        }

        stmt->name = ast::Identifier{this->curToken, this->curToken.literal};
        
        if (!this->expectPeek(token::ASSIGN)) {
            return {};
        }

        while (!this->curTokenIs(token::SEMICOLON)) {
            this->nextToken();
        }
        return stmt;
    }

    bool curTokenIs(std::string_view t) {
        return this->curToken.type == t;
    }

    bool peekTokenIs(std::string_view t) {
        return this->peekToken.type == t;
    }

    bool expectPeek(std::string_view t) {
        if (this->peekTokenIs(t)) {
            this->nextToken();
            return true;
        }
        return false;
    }

    void peekError(token::Token t) {
        this->errors.emplace_back(fmt::format("expected next token to be {}, got {} instead", t.type, this->peekToken.type));
    }

    lexer::Lexer l;
    token::Token curToken;
    token::Token peekToken;
    std::vector<std::string> errors;
};

#endif // parser_parser_h