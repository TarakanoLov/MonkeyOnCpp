#ifndef ast_ast_h
#define ast_ast_h

#include <iostream>
#include <vector>
#include <memory>

#include <token/token.h>

namespace ast {

struct Node {
    virtual std::string TokenLiteral() = 0;
    virtual std::string String() = 0;
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

    std::string String() {
        std::stringstream out;
        for (const auto& statement : statements) {
            out << statement->String();
        }
        return out.str();
    }

   std::vector<std::shared_ptr<Statement>> statements;
};

struct Identifier : public Expression {
    std::string expressionNode() override { return ""; }
    std::string TokenLiteral() {
        return this->token.literal;
    }
    std::string String() {
        return this->value;
    }

    token::Token token;
    std::string value;
};

struct LetStatement : public Statement {
    std::string statementNode() override { return ""; }
    std::string TokenLiteral() {
        return this->token.literal;
    }
    std::string String() override {
        std::stringstream out;
        out << this->TokenLiteral() << ' ' << name.String() << " = ";

        if (this->value) {
            out << this->value->String();
        }
        out << ';';
        return out.str();
    }

    token::Token token;
    Identifier name;
    std::shared_ptr<Expression> value;
};

struct ReturnStatement : public Statement {
    std::string statementNode() override { return ""; }
    std::string TokenLiteral() {
        return this->token.literal;
    }
    std::string String() override {
        std::stringstream out;


        out << this->TokenLiteral() << ' ';
        if (this->returnValue) {
            out << this->returnValue->String();
        }

        out << ';';
        return out.str();
    }

    token::Token token;
    std::shared_ptr<Expression> returnValue;
};

struct ExpressionStatement : public Statement {
    std::string statementNode() override { return ""; }
    std::string TokenLiteral() {
        return this->token.literal;
    }
    std::string String() override {
        if (this->expression.get()) {
            return this->expression->String();
        }
        return "";
    }

    token::Token token;
    std::shared_ptr<Expression> expression;
};

struct IntegerLiteral : public Expression {
    std::string expressionNode() override { return ""; }
    std::string TokenLiteral() {
        return this->token.literal;
    }
    std::string String() {
        return this->token.literal;
    }

    token::Token token;
    int64_t value{};
};

struct PrefixExpression : public Expression {
    std::string expressionNode() override { return ""; }
    std::string TokenLiteral() {
        return this->token.literal;
    }
    std::string String() {
        std::stringstream out;
        out << '(' << this->my_operator << this->right->String() << ')';
        return out.str();
    }

    token::Token token;
    std::string my_operator;
    std::shared_ptr<Expression> right;
};

struct InfixExpression : public Expression {
    std::string expressionNode() override { return ""; }
    std::string TokenLiteral() {
        return this->token.literal;
    }
    std::string String() {
        std::stringstream out;
        out << '(' << this->left->String() << ' ' << this->my_operator << ' ' << this->right->String() << ')';
        return out.str();
    }

    token::Token token;
    std::shared_ptr<Expression> left;
    std::string my_operator;
    std::shared_ptr<Expression> right;
};

struct Boolean : public Expression {
    std::string expressionNode() override { return ""; }
    std::string TokenLiteral() {
        return this->token.literal;
    }
    std::string String() {
        return this->token.literal;
    }

    token::Token token;
    bool value{};
};

struct BlockStatement : public Statement {
    std::string statementNode() override { return ""; }
    std::string TokenLiteral() {
        return this->token.literal;
    }
    std::string String() override {
        std::stringstream out;
        for (const auto& statement : statements) {
            out << statement->String();
        }
        return out.str();
    }

    token::Token token;
    std::vector<std::shared_ptr<Statement>> statements;
};

struct IfExpression : public Expression {
    std::string expressionNode() override { return ""; }
    std::string TokenLiteral() {
        return this->token.literal;
    }
    std::string String() {
        std::stringstream out;
        out << "if" << this->condition->String() << ' ' << this->consequence->String();
        if (this->alternative.get()) {
            out << "else " << this->alternative;
        }
        return out.str();
    }

    token::Token token;
    std::shared_ptr<Expression> condition;
    std::shared_ptr<BlockStatement> consequence;
    std::shared_ptr<BlockStatement> alternative;
};

} // namespace ast

#endif // ast_ast_h