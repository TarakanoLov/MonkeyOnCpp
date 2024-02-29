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

TEST(Lexer, My) {
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