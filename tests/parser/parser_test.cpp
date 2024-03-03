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

void testIntegerLiteral(std::shared_ptr<ast::Expression> il, int64_t value) {
    const auto integ = dynamic_cast<ast::IntegerLiteral*>(il.get());
    ASSERT_TRUE(!!integ);

    EXPECT_EQ(integ->value, value);

    EXPECT_EQ(integ->TokenLiteral(), std::to_string(value));
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

TEST(ParseProgram, PrefixExpressions) {
    const std::vector<std::tuple<std::string, std::string, int64_t>> prefixTests{{"!5;", "!", 5}, {"-15;", "-", 15}};
    for (const auto& one_test : prefixTests) {
        auto l = lexer::Lexer(std::get<0>(one_test));
        auto p = Parser(std::move(l));
        auto program = p.ParseProgram();
        checkParserError(p);

        EXPECT_EQ(program.statements.size(), 1);

        const auto stmt = dynamic_cast<ast::ExpressionStatement*>(program.statements[0].get());
        ASSERT_TRUE(!!stmt);

        const auto exp = dynamic_cast<ast::PrefixExpression*>(stmt->expression.get());
        ASSERT_TRUE(!!exp);

        EXPECT_EQ(exp->my_operator, std::get<1>(one_test));
        testIntegerLiteral(exp->right, std::get<2>(one_test));
    }
}

TEST(ParseProgram, InfixExpressions) {
    std::vector<std::tuple<std::string, int64_t, std::string, int64_t>> infixTests{{"5 + 5;", 5, "+", 5}, {"5 - 5;", 5, "-", 5},
{"5 * 5;", 5, "*", 5},
{"5 / 5;", 5, "/", 5},
{"5 > 5;", 5, ">", 5},
{"5 < 5;", 5, "<", 5},
{"5 == 5;", 5, "==", 5},
{"5 != 5;", 5, "!=", 5}};
    for (const auto& [input, leftValue, my_operator, rightValue] : infixTests) {
        auto l = lexer::Lexer(input);
        auto p = Parser(std::move(l));
        auto program = p.ParseProgram();
        checkParserError(p);

        ASSERT_EQ(program.statements.size(), 1);

        const auto stmt = dynamic_cast<ast::ExpressionStatement*>(program.statements[0].get());
        ASSERT_TRUE(!!stmt);

        const auto exp = dynamic_cast<ast::InfixExpression*>(stmt->expression.get());
        ASSERT_TRUE(!!exp);

        testIntegerLiteral(exp->left, leftValue);

        EXPECT_EQ(exp->my_operator, my_operator);

        testIntegerLiteral(exp->right, rightValue);
    }
}

TEST(ParseProgram, OperatorPrecedence) {
    std::vector<std::pair<std::string, std::string>> tests{{
"-a * b",
"((-a) * b)",
},
{
"!-a",
"(!(-a))",
},
{
"a + b + c",
"((a + b) + c)",
},
{
"a + b - c",
"((a + b) - c)",
},
{
"a * b * c",
"((a * b) * c)",
},
{
"a * b / c",
"((a * b) / c)",
},
{
"a + b / c",
"(a + (b / c))",
},
{
"a + b * c + d / e - f",
"(((a + (b * c)) + (d / e)) - f)",
},
{
"3 + 4; -5 * 5",
"(3 + 4)((-5) * 5)",
},
{
"5 > 4 == 3 < 4",
"((5 > 4) == (3 < 4))",
},
{
"5 < 4 != 3 > 4",
"((5 < 4) != (3 > 4))",
},
{
"3 + 4 * 5 == 3 * 1 + 4 * 5",
"((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
},
{
"3 + 4 * 5 == 3 * 1 + 4 * 5",
"((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))",
}};
    for (const auto& [input, exprected] : tests) {
        auto l = lexer::Lexer(input);
        auto p = Parser(std::move(l));
        auto program = p.ParseProgram();
        checkParserError(p);

        const auto actual = program.String();
        EXPECT_EQ(actual, exprected);
    }
}