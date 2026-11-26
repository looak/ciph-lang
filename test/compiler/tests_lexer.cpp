#include <gtest/gtest.h>

#include <ciph/compiler/lexer.hpp>
#include <ciph/compiler/token_cursor.hpp>

using namespace ciph;

TEST(LexerTest, PrimitiveTokens) {
    // setup
    Lexer lx("12345");
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    // do
    auto token = cursor.pop();

    // validate
    EXPECT_EQ(token.readStringValue(), "12345");
    EXPECT_EQ(token.readType(), TokenType::NUMBER);
}

bool
isOperator(const Token& token, OperatorType type) {
    return token.readType() == TokenType::OPERATOR && token.readOperatorValue() == type;
}

bool
OperatorTester(std::string input, OperatorType type) {
    // setup
    Lexer lx(input);
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    // do
    auto token = cursor.pop();

    // validate
    return isOperator(token, type);
}

TEST(LexerTest, IdentifyOperators) {
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

TEST(LexerTest, SimpleAddExpression) {
    // setup
    Lexer lx("1 +2");
    lx.lex();
    TokenCursor cursor(lx.readTokens());


    // do
    auto token1 = cursor.pop();
    auto token2 = cursor.pop();
    auto token3 = cursor.pop();

    // validate
    EXPECT_EQ(token1.readStringValue(), "1");
    EXPECT_EQ(token1.readType(), TokenType::NUMBER);

    EXPECT_TRUE(isOperator(token2, OperatorType::ADDITION));

    EXPECT_EQ(token3.readStringValue(), "2");
    EXPECT_EQ(token3.readType(), TokenType::NUMBER);
}

TEST(LexerTest, SimpleAssignmentExpression) {
    Lexer lx("myVar = 10 + 5 - (3 * 2)");
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    std::vector<Token> tokens;
    bool eofOrError = false;
    do {
        auto token = cursor.pop();
        tokens.push_back(token);
        if (token.readType() == TokenType::END_OF_FILE)
            eofOrError = true;
        else if (token.readType() == TokenType::UNKNOWN)
            eofOrError = true;
    } while (!eofOrError);

    EXPECT_EQ(tokens[0].readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].readStringValue(), "myVar");
    EXPECT_TRUE(isOperator(tokens[1], OperatorType::ASSIGNMENT));
    EXPECT_EQ(tokens[2].readType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[2].readStringValue(), "10");
    EXPECT_TRUE(isOperator(tokens[3], OperatorType::ADDITION));
    EXPECT_EQ(tokens[4].readType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[4].readStringValue(), "5");
    EXPECT_TRUE(isOperator(tokens[5], OperatorType::SUBTRACTION));
    EXPECT_EQ(tokens[6].readType(), TokenType::OPEN_PAREN);
    EXPECT_EQ(tokens[6].readStringValue(), "(");
    EXPECT_EQ(tokens[7].readType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[7].readStringValue(), "3");
    EXPECT_TRUE(isOperator(tokens[8], OperatorType::MULTIPLICATION));
    EXPECT_EQ(tokens[9].readType(), TokenType::NUMBER);
    EXPECT_EQ(tokens[9].readStringValue(), "2");
    EXPECT_EQ(tokens[10].readType(), TokenType::CLOSE_PAREN);
    EXPECT_EQ(tokens[10].readStringValue(), ")");
    EXPECT_EQ(tokens[11].readType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, KeywordsIdentification) {
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
        Lexer lx(kw.first);
        lx.lex();
        TokenCursor cursor(lx.readTokens());
        auto token = cursor.pop();
        EXPECT_EQ(token.readType(), kw.second) << "Failed for keyword: " << kw.first;
    }
}

TEST(LexerTest, IdentifierLiteral) {
    Lexer lx("myVar");
    lx.lex();
    TokenCursor cursor(lx.readTokens());
    auto token = cursor.pop();
    EXPECT_EQ(token.readStringValue(), "myVar");
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
}

TEST(LexerTest, LetExpression) {
    Lexer lx("let x = 5");
    lx.lex();
    TokenCursor cursor(lx.readTokens());
    auto token = cursor.pop();
    EXPECT_EQ(token.readStringValue(), "let");
    EXPECT_EQ(token.readType(), TokenType::LET);
    token = cursor.pop();
    EXPECT_EQ(token.readStringValue(), "x");
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_TRUE(isOperator(cursor.pop(), OperatorType::ASSIGNMENT));

    token = cursor.pop();
    EXPECT_EQ(token.readStringValue(), "5");
    EXPECT_EQ(token.readType(), TokenType::NUMBER);
}

TEST(LexerTest, BraceTokens) {
    Lexer lx("{");
    lx.lex();
    TokenCursor cursor(lx.readTokens());
    auto token = cursor.pop();
    EXPECT_EQ(token.readStringValue(), "{");
    EXPECT_EQ(token.readType(), TokenType::OPEN_BRACE);

    lx = Lexer("}");
    lx.lex();
    cursor = TokenCursor(lx.readTokens());
    token = cursor.pop();
    EXPECT_EQ(token.readStringValue(), "}");
    EXPECT_EQ(token.readType(), TokenType::CLOSE_BRACE);

    lx = Lexer("[");
    lx.lex();
    cursor = TokenCursor(lx.readTokens());
    token = cursor.pop();
    EXPECT_EQ(token.readStringValue(), "[");
    EXPECT_EQ(token.readType(), TokenType::OPEN_BRACKET);

    lx = Lexer("]");
    lx.lex();
    cursor = TokenCursor(lx.readTokens());
    token = cursor.pop();
    EXPECT_EQ(token.readStringValue(), "]");
    EXPECT_EQ(token.readType(), TokenType::CLOSE_BRACKET);
}

TEST(LexerTest, FunctionAndScope) {
    Lexer lx("fn myFunction() { return 0 }");
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    auto token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::FUNCTION);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readStringValue(), "myFunction");
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::OPEN_PAREN);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::CLOSE_PAREN);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::OPEN_BRACE);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::RETURN);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::NUMBER);
    EXPECT_EQ(token.readStringValue(), "0");
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::CLOSE_BRACE);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, FunctionAndScopeAndCall) {
    Lexer lx("fn myFunction() { return 0 } return myFunction()");
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    auto token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::FUNCTION);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readStringValue(), "myFunction");
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::OPEN_PAREN);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::CLOSE_PAREN);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::OPEN_BRACE);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::RETURN);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::NUMBER);
    EXPECT_EQ(token.readStringValue(), "0");
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::CLOSE_BRACE);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::RETURN);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readStringValue(), "myFunction");
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::OPEN_PAREN);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::CLOSE_PAREN);
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::END_OF_FILE);
}
TEST(LexerTest, WindowsLineEndings_AreWhitespace) {
    // setup
    Lexer lx("let x\r\nreturn");

    // do
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    // validate
    ASSERT_FALSE(lx.hasErrors());
    EXPECT_EQ(cursor.pop().readType(), TokenType::LET);
    EXPECT_EQ(cursor.pop().readType(), TokenType::IDENTIFIER);
    auto token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::RETURN);
    EXPECT_EQ(token.readPosition().line, 2u);
    EXPECT_EQ(cursor.pop().readType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, Tokens_KnowTheirColumnAndLength) {
    // setup:    123456789012345678
    Lexer lx("let count = 123 <= x;");

    // do
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    // validate
    ASSERT_FALSE(lx.hasErrors());
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
        auto token = cursor.pop();
        EXPECT_EQ(token.readPosition().column, e.column);
        EXPECT_EQ(token.readLength(), e.length);
    }
}

TEST(LexerTest, LineComments_AreSkipped) {
    // setup
    Lexer lx("let // the rest of this line is ignored: return 5 / 2\nx // comment at end of input");

    // do
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    // validate
    ASSERT_FALSE(lx.hasErrors());
    EXPECT_EQ(cursor.pop().readType(), TokenType::LET);
    auto token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readStringValue(), "x");
    EXPECT_EQ(token.readPosition().line, 2u);
    EXPECT_EQ(cursor.pop().readType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, Division_IsNotAComment) {
    // setup
    Lexer lx("6 / 2");

    // do
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    // validate
    ASSERT_FALSE(lx.hasErrors());
    cursor.pop(); // 6
    EXPECT_TRUE(isOperator(cursor.pop(), OperatorType::DIVISION));
    EXPECT_EQ(cursor.pop().readType(), TokenType::NUMBER);
}

TEST(LexerTest, FunctionSignatureTokens) {
    // setup
    Lexer lx("fn gcd_loop(mut a: i16, b: i16) -> i16");

    // do
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    // validate
    ASSERT_FALSE(lx.hasErrors());
    EXPECT_EQ(cursor.pop().readType(), TokenType::FUNCTION);

    auto token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readStringValue(), "gcd_loop");

    EXPECT_EQ(cursor.pop().readType(), TokenType::OPEN_PAREN);
    EXPECT_EQ(cursor.pop().readType(), TokenType::MUT);

    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readStringValue(), "a");

    EXPECT_EQ(cursor.pop().readType(), TokenType::COLON);

    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readStringValue(), "i16");

    EXPECT_EQ(cursor.pop().readType(), TokenType::COMMA);
    EXPECT_EQ(cursor.pop().readType(), TokenType::IDENTIFIER); // b
    EXPECT_EQ(cursor.pop().readType(), TokenType::COLON);
    EXPECT_EQ(cursor.pop().readType(), TokenType::IDENTIFIER); // i16
    EXPECT_EQ(cursor.pop().readType(), TokenType::CLOSE_PAREN);

    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::ARROW);
    EXPECT_EQ(token.readPosition().column, 33u);

    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readStringValue(), "i16");
    EXPECT_EQ(token.readPosition().column, 36u);

    EXPECT_EQ(cursor.pop().readType(), TokenType::END_OF_FILE);
}

TEST(LexerTest, MinusAndDecrement_AreNotArrows) {
    // setup
    Lexer lx("a - b a--");

    // do
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    // validate
    ASSERT_FALSE(lx.hasErrors());
    cursor.pop(); // a
    EXPECT_TRUE(isOperator(cursor.pop(), OperatorType::SUBTRACTION));
    cursor.pop(); // b
    cursor.pop(); // a
    EXPECT_TRUE(isOperator(cursor.pop(), OperatorType::DECREMENT));
}

TEST(LexerTest, Identifier_MayContainDigitsAndUnderscores) {
    // setup
    Lexer lx("_tmp x1 9lives");

    // do
    lx.lex();
    TokenCursor cursor(lx.readTokens());

    // validate
    ASSERT_FALSE(lx.hasErrors());
    auto token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readStringValue(), "_tmp");

    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::IDENTIFIER);
    EXPECT_EQ(token.readStringValue(), "x1");

    // A leading digit still starts a number.
    token = cursor.pop();
    EXPECT_EQ(token.readType(), TokenType::NUMBER);
    EXPECT_EQ(token.readStringValue(), "9");
}
