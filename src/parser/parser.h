#ifndef parser_parser_h
#define parser_parser_h

#include <functional>
#include <charconv>

#include <fmt/core.h>

#include <ast/ast.h>
#include <lexer/lexer.h>
#include <token/token.h>

struct Parser;

using prefixParseFn = std::function<std::shared_ptr<ast::Expression>(Parser*)>;
using infixParseFn = std::function<std::shared_ptr<ast::Expression>(Parser*, std::shared_ptr<ast::Expression>)>;

enum class Priority {
    Lowest,
    Equals,
    LessGreater,
    Sum,
    Product,
    Prefix,
    Call
};

inline const auto precedences = std::unordered_map<std::string_view, Priority>{
    {token::EQ, Priority::Equals},
    {token::NOT_EQ, Priority::Equals},
    {token::LT, Priority::LessGreater},
    {token::GT, Priority::LessGreater},
    {token::PLUS, Priority::Sum},
    {token::MINUS, Priority::Sum},
    {token::SLASH, Priority::Product},
    {token::ASTERISK, Priority::Product},
    };

struct Parser {
    Parser(lexer::Lexer lexer) : l(std::move(lexer)) {
        this->nextToken();
        this->nextToken();

        this->registerPrefix(token::IDENT, &Parser::parseIdentifier);
        this->registerPrefix(token::INT, &Parser::parseIntegerLiteral);
        this->registerPrefix(token::BANG, &Parser::parsePrefixExpression);
        this->registerPrefix(token::MINUS, &Parser::parsePrefixExpression);
    
        this->registerInfix(token::PLUS, &Parser::parseInfixExpression);
        this->registerInfix(token::MINUS, &Parser::parseInfixExpression);
        this->registerInfix(token::SLASH, &Parser::parseInfixExpression);
        this->registerInfix(token::ASTERISK, &Parser::parseInfixExpression);
        this->registerInfix(token::EQ, &Parser::parseInfixExpression);
        this->registerInfix(token::NOT_EQ, &Parser::parseInfixExpression);
        this->registerInfix(token::LT, &Parser::parseInfixExpression);
        this->registerInfix(token::GT, &Parser::parseInfixExpression);
    }

    std::shared_ptr<ast::Expression> parseIdentifier() {
        auto ident = std::make_shared<ast::Identifier>();
        ident->token = this->curToken;
        ident->value = this->curToken.literal;
        return ident;
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

    const std::vector<std::string>& Errors() const {
        return this->errors;
    }

    std::shared_ptr<ast::Statement> parseStatement() {
        if (this->curToken.type == token::LET) {
            return this->parseLetStatement();
        }
        if (this->curToken.type == token::RETURN) {
            return this->parseReturnStatement();
        }
        return this->parseExpressionStatement();
    }

    std::shared_ptr<ast::Statement> parseLetStatement() {
        auto stmt = std::make_shared<ast::LetStatement>();
        stmt->token = this->curToken;

        if (!this->expectPeek(token::IDENT)) {
            return {};
        }

        stmt->name = ast::Identifier{};
        stmt->name.token = this->curToken;
        stmt->name.value = this->curToken.literal;
        
        if (!this->expectPeek(token::ASSIGN)) {
            return {};
        }

        while (!this->curTokenIs(token::SEMICOLON)) {
            this->nextToken();
        }
        return stmt;
    }

    std::shared_ptr<ast::Statement> parseReturnStatement() {
        auto stmt = std::make_shared<ast::ReturnStatement>();
        stmt->token = this->curToken;

        this->nextToken();

        while (!this->curTokenIs(token::SEMICOLON)) {
            this->nextToken();
        }

        return stmt;
    }
    
    std::shared_ptr<ast::Statement> parseExpressionStatement() {
        auto stmt = std::make_shared<ast::ExpressionStatement>();
        stmt->token = this->curToken;

        stmt->expression = this->parseExpression(Priority::Lowest);

        if (this->peekTokenIs(token::SEMICOLON)) {
            this->nextToken();
        }

        return stmt;
    }

    std::shared_ptr<ast::Expression> parseExpression(Priority precedence) {
        auto prefix = this->prefixParseFns[this->curToken.type];
        if (!prefix) {
            this->noPrefixParseFnError(this->curToken.type);
            return {};
        }
        auto leftExp = std::invoke(prefix, this);

        while (!this->peekTokenIs(token::SEMICOLON) && precedence < this->peekPrecedence()) {
            auto infix = this->infixParseFns[this->peekToken.type];
            if (!infix) {
                return leftExp;
            }

            this->nextToken();

            leftExp = std::invoke(infix, this, leftExp);
        }
        return leftExp;
    }

    std::shared_ptr<ast::Expression> parseIntegerLiteral() {
        auto lit = std::make_shared<ast::IntegerLiteral>();
        lit->token = this->curToken;

        int64_t value{};
        std::from_chars(this->curToken.literal.data(), this->curToken.literal.data() + this->curToken.literal.size(), value);
        
        lit->value = value;
        return lit;
    }

    std::shared_ptr<ast::Expression> parsePrefixExpression() {
        auto expression = std::make_shared<ast::PrefixExpression>();
        expression->token = this->curToken;
        expression->my_operator = this->curToken.literal;

        this->nextToken();

        expression->right = this->parseExpression(Priority::Prefix);

        return expression;
    }

    std::shared_ptr<ast::Expression> parseInfixExpression(std::shared_ptr<ast::Expression> left) {
        auto expression = std::make_shared<ast::InfixExpression>();
        expression->token = this->curToken;
        expression->my_operator = this->curToken.literal;
        expression->left = left;

        const auto precedence = this->curPrecedence();
        this->nextToken();
        expression->right = this->parseExpression(precedence);

        return expression;
    }

    void noPrefixParseFnError(std::string_view t) {
        this->errors.emplace_back(fmt::format("no prefix parse function for {} found", t));
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
        this->peekError(t);
        return false;
    }

    void peekError(std::string_view t) {
        this->errors.emplace_back(fmt::format("expected next token to be {}, got {} instead", t, this->peekToken.type));
    }


    void registerPrefix(std::string tokenType, prefixParseFn fn) {
        this->prefixParseFns[std::move(tokenType)] = fn;
    }

    void registerInfix(std::string tokenType, infixParseFn fn) {
        this->infixParseFns[std::move(tokenType)] = fn;
    }

    Priority peekPrecedence() {
        const auto it = precedences.find(this->peekToken.type);
        if (it != precedences.end()) {
            return it->second;
        }
        return Priority::Lowest;
    }

    Priority curPrecedence() {
        const auto it = precedences.find(this->curToken.type);
        if (it != precedences.end()) {
            return it->second;
        }
        return Priority::Lowest;
    } 

    lexer::Lexer l;
    token::Token curToken;
    token::Token peekToken;
    std::vector<std::string> errors;

    std::unordered_map<std::string, prefixParseFn> prefixParseFns;
    std::unordered_map<std::string, infixParseFn> infixParseFns;
};

#endif // parser_parser_h