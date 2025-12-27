#include <gtest/gtest.h>

#include "lexar.hpp"

TEST(LexarTest, PrimitiveTokens) {
    // setup
    Lexar lx("12345");

    // do
    auto token = lx.pop();

    // validate
    EXPECT_EQ(token.readValue(), "12345");
    EXPECT_EQ(token.readType(), CIPHTokenType::NUMBER);
}

bool isOperator(const Token& token, OperatorType type) {
    return token.readType() == CIPHTokenType::OPERATOR && token.readOperator() == type;
}

bool OperatorTester(std::string input, OperatorType type) {
    // setup
    Lexar lx(input);

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

    // do
    auto token1 = lx.pop();
    auto token2 = lx.pop();
    auto token3 = lx.pop();

    // validate
    EXPECT_EQ(token1.readValue(), "1");
    EXPECT_EQ(token1.readType(), CIPHTokenType::NUMBER);

    EXPECT_TRUE(isOperator(token2, OperatorType::ADDITION));

    EXPECT_EQ(token3.readValue(), "2");
    EXPECT_EQ(token3.readType(), CIPHTokenType::NUMBER);
}

TEST(LexarTest, SimpleAssignmentExpression) {
    Lexar lx("myVar = 10 + 5 - (3 * 2)");

    std::vector<Token> tokens;
    bool eofOrError = false;
    do {
        auto token = lx.pop();
        tokens.push_back(token);
        if (token.readType() == CIPHTokenType::END_OF_FILE)
            eofOrError = true;
        else if (token.readType() == CIPHTokenType::UNKNOWN)
            eofOrError = true;
    } while (!eofOrError);

    EXPECT_EQ(tokens[0].readType(), CIPHTokenType::IDENTIFIER);
    EXPECT_EQ(tokens[0].readValue(), "myVar");
    EXPECT_TRUE(isOperator(tokens[1], OperatorType::ASSIGNMENT));
    EXPECT_EQ(tokens[2].readType(), CIPHTokenType::NUMBER);
    EXPECT_EQ(tokens[2].readValue(), "10");
    EXPECT_TRUE(isOperator(tokens[3], OperatorType::ADDITION));
    EXPECT_EQ(tokens[4].readType(), CIPHTokenType::NUMBER);
    EXPECT_EQ(tokens[4].readValue(), "5");
    EXPECT_TRUE(isOperator(tokens[5], OperatorType::SUBTRACTION));
    EXPECT_EQ(tokens[6].readType(), CIPHTokenType::OPEN_PAREN);
    EXPECT_EQ(tokens[6].readValue(), "(");
    EXPECT_EQ(tokens[7].readType(), CIPHTokenType::NUMBER);
    EXPECT_EQ(tokens[7].readValue(), "3");
    EXPECT_TRUE(isOperator(tokens[8], OperatorType::MULTIPLICATION));
    EXPECT_EQ(tokens[9].readType(), CIPHTokenType::NUMBER);
    EXPECT_EQ(tokens[9].readValue(), "2");
    EXPECT_EQ(tokens[10].readType(), CIPHTokenType::CLOSE_PAREN);
    EXPECT_EQ(tokens[10].readValue(), ")");
    EXPECT_EQ(tokens[11].readType(), CIPHTokenType::END_OF_FILE);
}

TEST(LexarTest, KeywordsIdentification) {
    std::vector<std::pair<std::string, CIPHTokenType>> keywordsToTest = {
        {"let", CIPHTokenType::LET},
        {"return", CIPHTokenType::RETURN},

        {"if", CIPHTokenType::IF},
        {"else", CIPHTokenType::ELSE},
        {"while", CIPHTokenType::WHILE}
        /*
        {"for", CIPHTokenType::FOR},
        {"break", CIPHTokenType::BREAK},
        {"continue", CIPHTokenType::CONTINUE},
        {"function", CIPHTokenType::FUNCTION},
        {"true", CIPHTokenType::TRUE},*/
    };

    for (auto& kw : keywordsToTest) {
        Lexar lx(kw.first);
        auto token = lx.pop();
        EXPECT_EQ(token.readType(), kw.second);
    }
}

TEST(LexarTest, IdentifierLiteral) {
    Lexar lx("myVar");
    auto token = lx.pop();
    EXPECT_EQ(token.readValue(), "myVar");
    EXPECT_EQ(token.readType(), CIPHTokenType::IDENTIFIER);
}

TEST(LexarTest, LetExpression) {
    Lexar lx("let x = 5");
    auto token = lx.pop();
    EXPECT_EQ(token.readValue(), "let");
    EXPECT_EQ(token.readType(), CIPHTokenType::LET);
    token = lx.pop();
    EXPECT_EQ(token.readValue(), "x");
    EXPECT_EQ(token.readType(), CIPHTokenType::IDENTIFIER);
    EXPECT_TRUE(isOperator(lx.pop(), OperatorType::ASSIGNMENT));

    token = lx.pop();
    EXPECT_EQ(token.readValue(), "5");
    EXPECT_EQ(token.readType(), CIPHTokenType::NUMBER);
}

TEST(LexarTest, BraceTokens) {
    Lexar lx("{");
    auto token = lx.pop();
    EXPECT_EQ(token.readValue(), "{");
    EXPECT_EQ(token.readType(), CIPHTokenType::OPEN_BRACE);

    lx = Lexar("}");
    token = lx.pop();
    EXPECT_EQ(token.readValue(), "}");
    EXPECT_EQ(token.readType(), CIPHTokenType::CLOSE_BRACE);

    lx = Lexar("[");
    token = lx.pop();
    EXPECT_EQ(token.readValue(), "[");
    EXPECT_EQ(token.readType(), CIPHTokenType::OPEN_BRACKET);

    lx = Lexar("]");
    token = lx.pop();
    EXPECT_EQ(token.readValue(), "]");
    EXPECT_EQ(token.readType(), CIPHTokenType::CLOSE_BRACKET);
}