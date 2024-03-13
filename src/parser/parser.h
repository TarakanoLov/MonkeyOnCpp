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
    {token::LPAREN, Priority::Call},
};

struct Parser {
    Parser(lexer::Lexer lexer) : l(std::move(lexer)) {
        this->nextToken();
        this->nextToken();

        this->registerPrefix(token::IDENT, &Parser::parseIdentifier);
        this->registerPrefix(token::INT, &Parser::parseIntegerLiteral);
        this->registerPrefix(token::BANG, &Parser::parsePrefixExpression);
        this->registerPrefix(token::MINUS, &Parser::parsePrefixExpression);
        this->registerPrefix(token::TRUE, &Parser::parseBoolean);
        this->registerPrefix(token::FALSE, &Parser::parseBoolean);
        this->registerPrefix(token::LPAREN, &Parser::parseGroupedExpression);
        this->registerPrefix(token::IF, &Parser::parseIfExpression);
        this->registerPrefix(token::FUNCTION, &Parser::parseFunctionLiteral);

        this->registerInfix(token::PLUS, &Parser::parseInfixExpression);
        this->registerInfix(token::MINUS, &Parser::parseInfixExpression);
        this->registerInfix(token::SLASH, &Parser::parseInfixExpression);
        this->registerInfix(token::ASTERISK, &Parser::parseInfixExpression);
        this->registerInfix(token::EQ, &Parser::parseInfixExpression);
        this->registerInfix(token::NOT_EQ, &Parser::parseInfixExpression);
        this->registerInfix(token::LT, &Parser::parseInfixExpression);
        this->registerInfix(token::GT, &Parser::parseInfixExpression);
        this->registerInfix(token::LPAREN, &Parser::parseCallExpression);
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

    std::shared_ptr<ast::Expression> parseBoolean() {
        auto expression = std::make_shared<ast::Boolean>();
        expression->token = this->curToken;
        expression->value = this->curTokenIs(token::TRUE);
        return expression;
    }

    std::shared_ptr<ast::Expression> parseGroupedExpression() {
        this->nextToken();

        auto exp = this->parseExpression(Priority::Lowest);
        if (!this->expectPeek(token::RPAREN)) {
            return {};
        }
        return exp;
    }

    std::shared_ptr<ast::BlockStatement> parseBlockStatement() {
        auto block = std::make_shared<ast::BlockStatement>();
        block->token = this->curToken;
        
        this->nextToken();

        while (!this->curTokenIs(token::RBRACE) && !this->curTokenIs(token::eof)) {
            auto stmt = this->parseStatement();
            if (stmt.get()) {
                block->statements.push_back(std::move(stmt));
            }
            this->nextToken();
        }
        return block;
    }

    std::shared_ptr<ast::Expression> parseIfExpression() {
        auto expression = std::make_shared<ast::IfExpression>();
        expression->token = this->curToken;

        if (!this->expectPeek(token::LPAREN)) {
            return {};
        }

        this->nextToken();
        expression->condition = this->parseExpression(Priority::Lowest);

        if (!this->expectPeek(token::RPAREN)) {
            return {};
        }

        if (!this->expectPeek(token::LBRACE)) {
            return {};
        }

        expression->consequence = this->parseBlockStatement();

        if (this->peekTokenIs(token::ELSE)) {
            this->nextToken();

            if (!this->expectPeek(token::LBRACE)) {
                return {};
            }

            expression->alternative = this->parseBlockStatement();
        }
        return expression;
    }

    std::vector<std::shared_ptr<ast::Identifier>> parseFunctionParameters() {
        std::vector<std::shared_ptr<ast::Identifier>> identifiers;
        if (this->peekTokenIs(token::RPAREN)) {
            this->nextToken();
            return identifiers;
        }

        this->nextToken();

        auto ident = std::make_shared<ast::Identifier>();
        ident->token = this->curToken;
        ident->value = this->curToken.literal;
        identifiers.push_back(std::move(ident));

        while (this->peekTokenIs(token::COMMA)) {
            this->nextToken();
            this->nextToken();
            ident = std::make_shared<ast::Identifier>();
            ident->token = this->curToken;
            ident->value = this->curToken.literal;
            identifiers.push_back(std::move(ident));
        }

        if (!this->expectPeek(token::RPAREN)) {
            return {};
        }

        return identifiers;
    }

    std::shared_ptr<ast::Expression> parseFunctionLiteral() {
        auto lit = std::make_shared<ast::FunctionLteral>();
        lit->token = this->curToken;

        if (!this->expectPeek(token::LPAREN)) {
            return {};
        }
        
        lit->parameters = this->parseFunctionParameters();

        if (!this->expectPeek(token::LBRACE)) {
            return {};
        }

        lit->body = this->parseBlockStatement();

        return lit;
    }

    std::vector<std::shared_ptr<ast::Expression>> parseCallArguments() {
        auto args = std::vector<std::shared_ptr<ast::Expression>>{};

        if (this->peekTokenIs(token::RPAREN)) {
            this->nextToken();
            return args;
        }

        this->nextToken();
        args.push_back(this->parseExpression(Priority::Lowest));

        while (this->peekTokenIs(token::COMMA)) {
            this->nextToken();
            this->nextToken();
            args.push_back(this->parseExpression(Priority::Lowest));
        }

        if (!this->expectPeek(token::RPAREN)) {
            return {};
        }

        return args;
    }

    std::shared_ptr<ast::Expression> parseCallExpression(std::shared_ptr<ast::Expression> function) {
        auto exp = std::make_shared<ast::CallExpression>();
        exp->token = this->curToken;
        exp->function = function;
        exp->arguments = this->parseCallArguments();
        return exp;
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