#include <gtest/gtest.h>

#include <ast/ast.h>

TEST(Program, String) {
    auto letStatement = std::make_shared<ast::LetStatement>();
    letStatement->token = token::Token(std::string{token::LET}, "let");
    letStatement->name.token = token::Token(token::IDENT, "myVar");
    letStatement->name.value = "myVar";

    auto identifier = std::make_shared<ast::Identifier>();
    identifier->token = token::Token(std::string{token::LET}, "anotherVar");
    identifier->value = "anotherVar";
    letStatement->value = std::move(identifier);
    
    auto program = ast::Program{};
    program.statements.emplace_back(std::move(letStatement)); 
    EXPECT_EQ(program.String(), "let myVar = anotherVar;");
}