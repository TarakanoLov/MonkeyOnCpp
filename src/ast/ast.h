#ifndef ast_ast_h
#define ast_ast_h

#include <iostream>
#include <vector>
#include <memory>

#include <token/token.h>

namespace ast {

struct Node {
    virtual std::string TokenLiteral() = 0;
};

struct Statement : public Node {
    virtual std::string statementNode() = 0;
};

struct Expression : public Node {
    virtual std::string expressionNode() = 0;
};

class Program {
public:
   std::string TokenLiteral() {
    if (this->statements.size() > 0) {
        return this->statements[0]->TokenLiteral();
    }
    return "";
   }

   std::vector<std::shared_ptr<Statement>> statements;
};

struct Identifier {
    void expressionNode() {}
    std::string TokenLiteral() {
        return this->token.literal;
    }

    token::Token token;
    std::string value;
};

struct LetStatement : public Statement {
    std::string statementNode() override { return ""; }
    std::string TokenLiteral() {
        return this->token.literal;
    }

    token::Token token;
    Identifier name;
    std::shared_ptr<Expression> expression;
};

} // namespace ast

#endif // ast_ast_h