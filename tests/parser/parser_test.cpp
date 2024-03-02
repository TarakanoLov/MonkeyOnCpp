#include <gtest/gtest.h>

#include <ast/ast.h>
#include <lexer/lexer.h>
#include <parser/parser.h>

void testLetStatement(std::shared_ptr<ast::Statement> s, std::string_view name) {
    EXPECT_EQ(s->TokenLiteral(), "let");

    const auto letStmt = dynamic_cast<ast::LetStatement*>(s.get());
    EXPECT_TRUE(!!letStmt);

    EXPECT_EQ(letStmt->name.value, name);

    EXPECT_EQ(letStmt->name.TokenLiteral(), name);
}

void checkParserError(const Parser& p) {
    const auto& errors = p.Errors();

    if (errors.empty()) {
        return;
    }

    EXPECT_TRUE(false) << "parser has " << errors.size() << " errors";
    for (const auto& error : errors) {
        EXPECT_TRUE(false) << "parser error: " << error;
    }
}

TEST(Parser, Let) {
    const std::string input = R"(
        let x = 5;
        let y = 10;
        let foobar = 838383;
    )";

    auto l = lexer::Lexer(input);
    auto p = Parser(std::move(l));
    auto program = p.ParseProgram();
    checkParserError(p);

    ASSERT_EQ(program.statements.size(), 3);
    std::vector<std::string> tests{"x", "y", "foobar"};

    for (int i = 0; i < tests.size(); ++i) {
        testLetStatement(program.statements[i], tests[i]);
    }
}

TEST(Parser, Return) {
    const std::string input = R"(
        return 5;
        return 10;
        return 993322;
    )";

    auto l = lexer::Lexer(input);
    auto p = Parser(std::move(l));
    auto program = p.ParseProgram();
    checkParserError(p);

    ASSERT_EQ(program.statements.size(), 3);

    for (int i = 0; i < program.statements.size(); ++i) {
        const auto returnStmt = dynamic_cast<ast::ReturnStatement*>(program.statements[i].get());
        ASSERT_TRUE(!!returnStmt);

        EXPECT_EQ(returnStmt->TokenLiteral(), "return");
    }
}

TEST(ParseProgram, Identifier) {
    const std::string input = "foobar;";

    auto l = lexer::Lexer(input);
    auto p = Parser(std::move(l));
    auto program = p.ParseProgram();
    checkParserError(p);

    ASSERT_EQ(program.statements.size(), 1);

    const auto stmt = dynamic_cast<ast::ExpressionStatement*>(program.statements[0].get());
    ASSERT_TRUE(!!stmt);

    const auto ident = dynamic_cast<ast::Identifier*>(stmt->expression.get());
    ASSERT_TRUE(!!ident);

    EXPECT_EQ(ident->value, "foobar");

    EXPECT_EQ(ident->TokenLiteral(), "foobar");
}

TEST(ParseProgram, IntegerLiteral) {
    const std::string input = "5;";

    auto l = lexer::Lexer(input);
    auto p = Parser(std::move(l));
    auto program = p.ParseProgram();
    checkParserError(p);

    ASSERT_EQ(program.statements.size(), 1);

    const auto stmt = dynamic_cast<ast::ExpressionStatement*>(program.statements[0].get());
    ASSERT_TRUE(!!stmt);

    const auto literal = dynamic_cast<ast::IntegerLiteral*>(stmt->expression.get());
    ASSERT_TRUE(!!literal);

    EXPECT_EQ(literal->value, 5);
    EXPECT_EQ(literal->TokenLiteral(), "5");
}