#include <gtest/gtest.h>

#include <ciph/compiler/lexar.hpp>

using namespace ciph;

TEST(LexarTest, PrimitiveTokens) {
    // setup
    Lexar lx("12345");
    lx.lex();

    // do
    auto token = lx.pop();

    // validate
    EXPECT_EQ(token.readValue(), "12345");
    EXPECT_EQ(token.readType(), TokenType::NUMBER);
}

bool
isOperator(const Token& token, OperatorType type) {
    return token.readType() == TokenType::OPERATOR && token.readOperator() == type;
}

bool
OperatorTester(std::string input, OperatorType type) {
    // setup
    Lexar lx(input);
    lx.lex();

    // do
    auto token = lx.pop();

    // validate
    return isOperator(token, type);
}

TEST(LexarTest, IdentifyOperators) {
    std::vector<std::pair<std::string, OperatorType>> operatorsToTest = {
        {"=", OperatorType::ASSIGNMENT},
        {"==", OperatorType::EQUAL},
        {"!=", OperatorType::NOT_EQUAL},
        {"!", OperatorType::NOT},
        {"<", OperatorType::LESS_THAN},
        {">", OperatorType::GREATER_THAN},
        {"<=", OperatorType::LESS_EQUAL},
        {">=", OperatorType::GREATER_EQUAL},
        {"&&", OperatorType::AND_AND},
        {"&", OperatorType::AND},
        {"||", OperatorType::OR_OR},
        {"|", OperatorType::OR},
        {"^", OperatorType::XOR},
        {"%", OperatorType::MODULUS},
        {"%=", OperatorType::MODULUS_ASSIGNMENT},
        {"*", OperatorType::MULTIPLICATION},
        {"*=", OperatorType::MULTIPLICATION_ASSIGNMENT},
        {"/", OperatorType::DIVISION},
        {"/=", OperatorType::DIVISION_ASSIGNMENT},
        {"<<", OperatorType::LEFT_SHIFT},
        //{ "<<=",	OperatorType::LEFT_SHIFT_ASSIGNMENT},
        {">>", OperatorType::RIGHT_SHIFT},
        //{ ">>=",	OperatorType::RIGHT_SHIFT_ASSIGNMENT},
        {"&", OperatorType::AND},
        {"&=", OperatorType::AND_ASSIGNMENT},
        {"|", OperatorType::OR},
        {"|=", OperatorType::OR_ASSIGNMENT},
        {"^", OperatorType::XOR},
        {"^=", OperatorType::XOR_ASSIGNMENT},
        {"++", OperatorType::INCREMENT},
        {"+", OperatorType::ADDITION},
        {"+=", OperatorType::ADDITION_ASSIGNMENT},
        {"++", OperatorType::INCREMENT},
        {"-", OperatorType::SUBTRACTION},
        {"-=", OperatorType::SUBTRACTION_ASSIGNMENT},
        {"--", OperatorType::DECREMENT},
    };

    for (auto& op : operatorsToTest) {
        EXPECT_TRUE(OperatorTester(op.first, op.second)) << "Failed for operator: " << op.first;
    }
}

TEST(LexarTest, SimpleAddExpression) {
    // setup
    Lexar lx("1 +2");
    lx.lex();


    // do
    auto token1 = lx.pop();
    auto token2 = lx.pop();
    auto token3 = lx.pop();

    // validate
    EXPECT_EQ(token1.readValue(), "1");
    EXPECT_EQ(token1.readType(), TokenType::NUMBER);

    EXPECT_TRUE(isOperator(token2, OperatorType::ADDITION));

    EXPECT_EQ(token3.readValue(), "2");
    EXPECT_EQ(token3.readType(), TokenType::NUMBER);
}

TEST(LexarTest, SimpleAssignmentExpression) {
    Lexar lx("myVar = 10 + 5 - (3 * 2)");
    lx.lex();

    std::vector<Token> tokens;
    bool eofOrError = false;
    do {
        auto token = lx.pop();
        tokens.push_back(token);
        if (token.readType() == TokenType::END_OF_FILE)
            eofOrError = true;
        else if (token.readType() == TokenType::UNKNOWN)
            eofOrError = true;
    } while (!eofOrError);

    EXPECT_EQ(tokens[0].readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].readValue(), "myVar");
    EXPECT_TRUE(isOperator(tokens[1], OperatorType::ASSIGNMENT));
    EXPECT_EQ(tokens[2].readType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[2].readValue(), "10");
    EXPECT_TRUE(isOperator(tokens[3], OperatorType::ADDITION));
    EXPECT_EQ(tokens[4].readType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[4].readValue(), "5");
    EXPECT_TRUE(isOperator(tokens[5], OperatorType::SUBTRACTION));
    EXPECT_EQ(tokens[6].readType(), TokenType::OPEN_PAREN);
    EXPECT_EQ(tokens[6].readValue(), "(");
    EXPECT_EQ(tokens[7].readType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[7].readValue(), "3");
    EXPECT_TRUE(isOperator(tokens[8], OperatorType::MULTIPLICATION));
    EXPECT_EQ(tokens[9].readType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[9].readValue(), "2");
    EXPECT_EQ(tokens[10].readType(), TokenType::CLOSE_PAREN);
    EXPECT_EQ(tokens[10].readValue(), ")");
    EXPECT_EQ(tokens[11].readType(), TokenType::END_OF_FILE);
}

TEST(LexarTest, KeywordsIdentification) {
    std::vector<std::pair<std::string, TokenType>> keywordsToTest = {
        {"let", TokenType::LET},
        {"return", TokenType::RETURN},

        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"fn", TokenType::FUNCTION}
        /*
        {"for", TokenType::FOR},
        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
        {"true", TokenType::TRUE},*/
    };

    for (auto& kw : keywordsToTest) {
        Lexar lx(kw.first);
        lx.lex();
        auto token = lx.pop();
        EXPECT_EQ(token.readType(), kw.second) << "Failed for keyword: " << kw.first;
    }
}

TEST(LexarTest, IdentifierLiteral) {
    Lexar lx("myVar");
    lx.lex();
    auto token = lx.pop();
    EXPECT_EQ(token.readValue(), "myVar");
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
}

TEST(LexarTest, LetExpression) {
    Lexar lx("let x = 5");
    lx.lex();
    auto token = lx.pop();
    EXPECT_EQ(token.readValue(), "let");
    EXPECT_EQ(token.readType(), TokenType::LET);
    token = lx.pop();
    EXPECT_EQ(token.readValue(), "x");
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_TRUE(isOperator(lx.pop(), OperatorType::ASSIGNMENT));

    token = lx.pop();
    EXPECT_EQ(token.readValue(), "5");
    EXPECT_EQ(token.readType(), TokenType::NUMBER);
}

TEST(LexarTest, BraceTokens) {
    Lexar lx("{");
    lx.lex();
    auto token = lx.pop();
    EXPECT_EQ(token.readValue(), "{");
    EXPECT_EQ(token.readType(), TokenType::OPEN_BRACE);

    lx = Lexar("}");
    lx.lex();
    token = lx.pop();
    EXPECT_EQ(token.readValue(), "}");
    EXPECT_EQ(token.readType(), TokenType::CLOSE_BRACE);

    lx = Lexar("[");
    lx.lex();
    token = lx.pop();
    EXPECT_EQ(token.readValue(), "[");
    EXPECT_EQ(token.readType(), TokenType::OPEN_BRACKET);

    lx = Lexar("]");
    lx.lex();
    token = lx.pop();
    EXPECT_EQ(token.readValue(), "]");
    EXPECT_EQ(token.readType(), TokenType::CLOSE_BRACKET);
}

TEST(LexarTest, FunctionAndScope) {
    Lexar lx("fn myFunction() { return 0 }");
    lx.lex();

    auto token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::FUNCTION);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readValue(), "myFunction");
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::OPEN_PAREN);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::CLOSE_PAREN);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::OPEN_BRACE);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::RETURN);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::NUMBER);
    EXPECT_EQ(token.readValue(), "0");
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::CLOSE_BRACE);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::END_OF_FILE);
}

TEST(LexarTest, FunctionAndScopeAndCall) {
    Lexar lx("fn myFunction() { return 0 } return myFunction()");
    lx.lex();

    auto token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::FUNCTION);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readValue(), "myFunction");
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::OPEN_PAREN);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::CLOSE_PAREN);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::OPEN_BRACE);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::RETURN);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::NUMBER);
    EXPECT_EQ(token.readValue(), "0");
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::CLOSE_BRACE);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::RETURN);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readValue(), "myFunction");
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::OPEN_PAREN);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::CLOSE_PAREN);
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::END_OF_FILE);
}
TEST(LexarTest, WindowsLineEndings_AreWhitespace) {
    // setup
    Lexar lx("let x\r\nreturn");

    // do
    auto [success, error] = lx.lex();

    // validate
    ASSERT_TRUE(success);
    EXPECT_EQ(lx.pop().readType(), TokenType::LET);
    EXPECT_EQ(lx.pop().readType(), TokenType::IDENTIFIER);
    auto token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::RETURN);
    EXPECT_EQ(token.readPosition().line, 2u);
    EXPECT_EQ(lx.pop().readType(), TokenType::END_OF_FILE);
}

TEST(LexarTest, Tokens_KnowTheirColumnAndLength) {
    // setup:    123456789012345678
    Lexar lx("let count = 123 <= x;");

    // do
    auto [success, error] = lx.lex();

    // validate
    ASSERT_TRUE(success);
    struct Expected {
        uint32_t column;
        uint32_t length;
    };
    const Expected expected[] = {
        {1, 3},  // let
        {5, 5},  // count
        {11, 1}, // =
        {13, 3}, // 123
        {17, 2}, // <=  starts at its first character
        {20, 1}, // x
        {21, 1}, // ;
        {22, 0}, // end of file
    };
    for (const Expected& e : expected) {
        auto token = lx.pop();
        EXPECT_EQ(token.readPosition().column, e.column);
        EXPECT_EQ(token.readLength(), e.length);
    }
}

TEST(LexarTest, LineComments_AreSkipped) {
    // setup
    Lexar lx("let // the rest of this line is ignored: return 5 / 2\nx // comment at end of input");

    // do
    auto [success, error] = lx.lex();

    // validate
    ASSERT_TRUE(success);
    EXPECT_EQ(lx.pop().readType(), TokenType::LET);
    auto token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readValue(), "x");
    EXPECT_EQ(token.readPosition().line, 2u);
    EXPECT_EQ(lx.pop().readType(), TokenType::END_OF_FILE);
}

TEST(LexarTest, Division_IsNotAComment) {
    // setup
    Lexar lx("6 / 2");

    // do
    auto [success, error] = lx.lex();

    // validate
    ASSERT_TRUE(success);
    lx.pop(); // 6
    EXPECT_TRUE(isOperator(lx.pop(), OperatorType::DIVISION));
    EXPECT_EQ(lx.pop().readType(), TokenType::NUMBER);
}

TEST(LexarTest, FunctionSignatureTokens) {
    // setup
    Lexar lx("fn gcd_loop(mut a: i16, b: i16) -> i16");

    // do
    auto [success, error] = lx.lex();

    // validate
    ASSERT_TRUE(success);
    EXPECT_EQ(lx.pop().readType(), TokenType::FUNCTION);

    auto token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readValue(), "gcd_loop");

    EXPECT_EQ(lx.pop().readType(), TokenType::OPEN_PAREN);
    EXPECT_EQ(lx.pop().readType(), TokenType::MUT);

    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readValue(), "a");

    EXPECT_EQ(lx.pop().readType(), TokenType::COLON);

    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readValue(), "i16");

    EXPECT_EQ(lx.pop().readType(), TokenType::COMMA);
    EXPECT_EQ(lx.pop().readType(), TokenType::IDENTIFIER); // b
    EXPECT_EQ(lx.pop().readType(), TokenType::COLON);
    EXPECT_EQ(lx.pop().readType(), TokenType::IDENTIFIER); // i16
    EXPECT_EQ(lx.pop().readType(), TokenType::CLOSE_PAREN);

    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::ARROW);
    EXPECT_EQ(token.readPosition().column, 33u);

    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readValue(), "i16");
    EXPECT_EQ(token.readPosition().column, 36u);

    EXPECT_EQ(lx.pop().readType(), TokenType::END_OF_FILE);
}

TEST(LexarTest, MinusAndDecrement_AreNotArrows) {
    // setup
    Lexar lx("a - b a--");

    // do
    auto [success, error] = lx.lex();

    // validate
    ASSERT_TRUE(success);
    lx.pop(); // a
    EXPECT_TRUE(isOperator(lx.pop(), OperatorType::SUBTRACTION));
    lx.pop(); // b
    lx.pop(); // a
    EXPECT_TRUE(isOperator(lx.pop(), OperatorType::DECREMENT));
}

TEST(LexarTest, Identifier_MayContainDigitsAndUnderscores) {
    // setup
    Lexar lx("_tmp x1 9lives");

    // do
    auto [success, error] = lx.lex();

    // validate
    ASSERT_TRUE(success);
    auto token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readValue(), "_tmp");

    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readValue(), "x1");

    // A leading digit still starts a number.
    token = lx.pop();
    EXPECT_EQ(token.readType(), TokenType::NUMBER);
    EXPECT_EQ(token.readValue(), "9");
}
