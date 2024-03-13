#include <ranges>

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

void testIdentifier(std::shared_ptr<ast::Expression> exp, std::string_view value) {
    const auto ident = dynamic_cast<ast::Identifier*>(exp.get());
    ASSERT_TRUE(!!ident);

    EXPECT_EQ(ident->value, value);
    EXPECT_EQ(ident->TokenLiteral(), value);
}

void testBooleanLiteral(std::shared_ptr<ast::Expression> exp, bool value) {
    const auto bo = dynamic_cast<ast::Boolean*>(exp.get());
    ASSERT_TRUE(!!bo);

    EXPECT_EQ(bo->value, value);

    EXPECT_EQ(bo->TokenLiteral(), (value ? "true" : "false"));
}

template <typename T>
void testLiteralExpression(std::shared_ptr<ast::Expression> exp, T value) {
    if constexpr (std::is_same_v<T, int>) {
        return testIntegerLiteral(exp, value);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        return testIntegerLiteral(exp, value);
    } else if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, const char*> || std::is_same_v<T, std::string>) {
        return testIdentifier(exp, value);
    } else if constexpr (std::is_same_v<T, bool>) {
        return testBooleanLiteral(exp, value);
    }
    EXPECT_TRUE(false) << "value = " << value << "with type = " << typeid(T).name();
}

template <typename Left, typename Right>
void testInfixExpression(std::shared_ptr<ast::Expression> exp, Left left, std::string_view cur_operator, Right right) {
    const auto opExpr = dynamic_cast<ast::InfixExpression*>(exp.get());
    ASSERT_TRUE(!!opExpr);
    
    testLiteralExpression(opExpr->left, left);

    EXPECT_EQ(opExpr->my_operator, cur_operator);

    testLiteralExpression(opExpr->right, right);
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
    const std::vector<std::tuple<std::string, std::string, bool>> prefixTestsBool{{"!true;", "!", true},
{"!false;", "!", false}};
    for (const auto& one_test : prefixTestsBool) {
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
        testLiteralExpression(exp->right, std::get<2>(one_test));
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
std::vector<std::tuple<std::string, bool, std::string, bool>> infixTestsBool{{"true == true", true, "==", true},
{"true != false", true, "!=", false},
{"false == false", false, "==", false}};
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

        testLiteralExpression(exp->left, leftValue);

        EXPECT_EQ(exp->my_operator, my_operator);

        testLiteralExpression(exp->right, rightValue);
    }

    for (const auto& [input, leftValue, my_operator, rightValue] : infixTestsBool) {
        auto l = lexer::Lexer(input);
        auto p = Parser(std::move(l));
        auto program = p.ParseProgram();
        checkParserError(p);

        ASSERT_EQ(program.statements.size(), 1);

        const auto stmt = dynamic_cast<ast::ExpressionStatement*>(program.statements[0].get());
        ASSERT_TRUE(!!stmt);

        const auto exp = dynamic_cast<ast::InfixExpression*>(stmt->expression.get());
        ASSERT_TRUE(!!exp);

        testLiteralExpression(exp->left, leftValue);

        EXPECT_EQ(exp->my_operator, my_operator);

        testLiteralExpression(exp->right, rightValue);
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
},
{
    "true", "true"
},
{
    "false", "false"
},
{
    "3 > 5 == false",
    "((3 > 5) == false)"
},
{
    "3 < 5 == true",
    "((3 < 5) == true)"
},
{
"1 + (2 + 3) + 4",
"((1 + (2 + 3)) + 4)",
},
{
"(5 + 5) * 2",
"((5 + 5) * 2)",
},
{
"2 / (5 + 5)",
"(2 / (5 + 5))",
},
{
"-(5 + 5)",
"(-(5 + 5))",
},
{
"!(true == true)",
"(!(true == true))",
},
{
"a + add(b * c) + d",
"((a + add((b * c))) + d)",
},
{
"add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
"add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))",
},
{
"add(a + b + c * d / f + g)",
"add((((a + b) + ((c * d) / f)) + g))",
}
};
    for (const auto& [input, exprected] : tests) {
        auto l = lexer::Lexer(input);
        auto p = Parser(std::move(l));
        auto program = p.ParseProgram();
        checkParserError(p);

        const auto actual = program.String();
        EXPECT_EQ(actual, exprected);
    }
}

TEST(ParseProgram, IfExpression) {
    const std::string input = "if (x < y) { x }";

    auto l = lexer::Lexer(input);
    auto p = Parser(std::move(l));
    auto program = p.ParseProgram();
    checkParserError(p);

    ASSERT_EQ(program.statements.size(), 1);

    const auto stmt = dynamic_cast<ast::ExpressionStatement*>(program.statements[0].get());
    ASSERT_TRUE(!!stmt);

    const auto exp = dynamic_cast<ast::IfExpression*>(stmt->expression.get());
    ASSERT_TRUE(!!exp);

    testInfixExpression(exp->condition, std::string_view{"x"}, "<", std::string_view{"y"});

    EXPECT_EQ(exp->consequence->statements.size(), 1);

    const auto concequence = dynamic_cast<ast::ExpressionStatement*>(exp->consequence->statements[0].get());
    ASSERT_TRUE(!!concequence);

    testIdentifier(concequence->expression, std::string_view{"x"});

    EXPECT_FALSE(exp->alternative.get());
}

TEST(ParseProgram, IfElseExpression) {
    const std::string input = "if (x < y) { x } else { y }";

    auto l = lexer::Lexer(input);
    auto p = Parser(std::move(l));
    auto program = p.ParseProgram();
    checkParserError(p);

    ASSERT_EQ(program.statements.size(), 1);

    const auto stmt = dynamic_cast<ast::ExpressionStatement*>(program.statements[0].get());
    ASSERT_TRUE(!!stmt);

    const auto exp = dynamic_cast<ast::IfExpression*>(stmt->expression.get());
    ASSERT_TRUE(!!exp);

    testInfixExpression(exp->condition, std::string_view{"x"}, "<", std::string_view{"y"});

    EXPECT_EQ(exp->consequence->statements.size(), 1);

    const auto concequence = dynamic_cast<ast::ExpressionStatement*>(exp->consequence->statements[0].get());
    ASSERT_TRUE(!!concequence);

    testIdentifier(concequence->expression, std::string_view{"x"});

    EXPECT_TRUE(exp->alternative.get());
    const auto alternative = dynamic_cast<ast::ExpressionStatement*>(exp->alternative->statements[0].get());
    ASSERT_TRUE(!!concequence);

    testIdentifier(alternative->expression, std::string_view{"y"});
}

TEST(ParseProgram, FunctionLiteral) {
    const std::string input = "fn(x, y) { x + y; }";

    auto l = lexer::Lexer(input);
    auto p = Parser(std::move(l));
    auto program = p.ParseProgram();
    checkParserError(p);

    ASSERT_EQ(program.statements.size(), 1);

    const auto stmt = dynamic_cast<ast::ExpressionStatement*>(program.statements[0].get());
    ASSERT_TRUE(!!stmt);

    const auto function = dynamic_cast<ast::FunctionLteral*>(stmt->expression.get());
    ASSERT_TRUE(!!function);

    ASSERT_EQ(function->parameters.size(), 2);

    testLiteralExpression(function->parameters[0], "x");
    testLiteralExpression(function->parameters[1], "y");

    ASSERT_EQ(function->body->statements.size(), 1);

    const auto bodyStmt = dynamic_cast<ast::ExpressionStatement*>(function->body->statements[0].get());
    ASSERT_TRUE(!!bodyStmt);

    testInfixExpression(bodyStmt->expression, "x", "+", "y");
}

TEST(ParseProgram, FunctionParameter) {
    const std::vector<std::pair<std::string, std::vector<std::string>>> tests {
        {"fn() {};", {}},
        {"fn(x) {};", std::vector<std::string>{"x"}},
        {"fn(x, y, z) {}", std::vector<std::string>{"x", "y", "z"}}
    };

    for (const auto& [input, expectedParams] : tests) {
        auto l = lexer::Lexer(input);
        auto p = Parser(std::move(l));
        auto program = p.ParseProgram();
        checkParserError(p);

        const auto stmt = dynamic_cast<ast::ExpressionStatement*>(program.statements[0].get());
        ASSERT_TRUE(!!stmt);
        const auto function = dynamic_cast<ast::FunctionLteral*>(stmt->expression.get());
        ASSERT_TRUE(!!function);

        ASSERT_EQ(function->parameters.size(), expectedParams.size());

        for (uint32_t i = 0; i < expectedParams.size(); ++i) {
            testLiteralExpression(function->parameters[i], expectedParams[i]);
        }
    }
}

TEST(ParseProgram, CallExpression) {
    const std::string input = "add(1, 2 * 3, 4 + 5)";
    auto l = lexer::Lexer(input);
    auto p = Parser(std::move(l));
    auto program = p.ParseProgram();
    checkParserError(p);

    ASSERT_EQ(program.statements.size(), 1);

    const auto stmt = dynamic_cast<ast::ExpressionStatement*>(program.statements[0].get());
    ASSERT_TRUE(!!stmt);

    const auto exp = dynamic_cast<ast::CallExpression*>(stmt->expression.get());
    ASSERT_TRUE(!!exp);

    testIdentifier(exp->function, "add");

    ASSERT_EQ(exp->arguments.size(), 3);

    testLiteralExpression(exp->arguments[0], 1);
    testInfixExpression(exp->arguments[1], 2, "*", 3);
    testInfixExpression(exp->arguments[2], 4, "+", 5);
}