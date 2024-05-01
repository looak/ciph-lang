#include <fmt/core.h>
#include <gtest/gtest.h>

#include "ast.hpp"
#include "error_defines.hpp"
#include "parser.hpp"

TEST(ParserTest, SimpleAddExpression) {
    // setup
    std::string code("1 + 2");
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto program_node = std::get<ASTBaseNode*>(parser_result);
    ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

    EXPECT_EQ(result->readStatements().size(), 1);
    ASSERT_EQ(result->readStatements()[0]->readType(), ASTNodeType::BINARY_EXPRESSION);
    auto binaryExpression = static_cast<ASTBinaryExpressionNode*>(result->readStatements()[0]);
    EXPECT_EQ(binaryExpression->readOperator(), OperatorType::ADDITION);
}

TEST(ParserTest, OrderOfOperations_MulBeforeAdd) {
    // setup
    std::string code("2 * 2 + 3");
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto program_node = std::get<ASTBaseNode*>(parser_result);
    ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

    EXPECT_EQ(result->readStatements().size(), 1);
    EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::BINARY_EXPRESSION);
    auto addExpression = static_cast<const ASTBinaryExpressionNode*>(result->readStatements()[0]);
    EXPECT_EQ(addExpression->readOperator(), OperatorType::ADDITION);

    auto left = addExpression->readLeft();
    EXPECT_EQ(left->readType(), ASTNodeType::BINARY_EXPRESSION);
    auto mulExpression = static_cast<const ASTBinaryExpressionNode*>(left);
    EXPECT_EQ(mulExpression->readOperator(), OperatorType::MULTIPLICATION);
}

TEST(ParserTest, OrderOfOperations_ParenthaseseBeforeMul) {
    // setup
    std::string code("2 * (2 + 3)");
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto program_node = std::get<ASTBaseNode*>(parser_result);
    ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

    EXPECT_EQ(result->readStatements().size(), 1);
    EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::BINARY_EXPRESSION);
    auto mulExpression = static_cast<const ASTBinaryExpressionNode*>(result->readStatements()[0]);
    EXPECT_EQ(mulExpression->readOperator(), OperatorType::MULTIPLICATION);

    auto right = mulExpression->readRight();
    EXPECT_EQ(right->readType(), ASTNodeType::BINARY_EXPRESSION);
    auto addExpression = static_cast<const ASTBinaryExpressionNode*>(right);
    EXPECT_EQ(addExpression->readOperator(), OperatorType::ADDITION);
}

TEST(ParserTest, ReturnStatement_DefaultNumericNode_ValueZero) {
    // setup
    std::string code("return");
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto program_node = std::get<ASTBaseNode*>(parser_result);
    ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

    EXPECT_EQ(result->readStatements().size(), 1);
    EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);
    auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);

    EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::NUMERIC_LITERAL);
    auto numericNode = static_cast<const ASTNumericLiteralNode*>(returnNode->readExpression());
    EXPECT_EQ(numericNode->readValue(), 0);
}

TEST(ParserTest, ReturnStatement_NumericNodeGivenValue) {
    // setup
    std::string code("return 5");
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto program_node = std::get<ASTBaseNode*>(parser_result);
    ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

    EXPECT_EQ(result->readStatements().size(), 1);
    EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);
    auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);

    EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::NUMERIC_LITERAL);
    auto numericNode = static_cast<const ASTNumericLiteralNode*>(returnNode->readExpression());
    EXPECT_EQ(numericNode->readValue(), 5);
}

TEST(ParserTest, ReturnStatement_NumericNodeExpression) {
    // setup
    std::string code("return 5 + 2");
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto program_node = std::get<ASTBaseNode*>(parser_result);
    ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

    EXPECT_EQ(result->readStatements().size(), 1);
    EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);
    auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);

    EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::BINARY_EXPRESSION);
}

TEST(ParserTest, LetStatement) {
    // setup
    std::string code("let x = 5");
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto program_node = std::get<ASTBaseNode*>(parser_result);
    ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

    EXPECT_EQ(result->readStatements().size(), 1);
    EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::LET);
    auto letNode = static_cast<const ASTLetNode*>(result->readStatements()[0]);
    EXPECT_EQ(letNode->readExpression()->readType(), ASTNodeType::NUMERIC_LITERAL);
    EXPECT_EQ(letNode->readIdentifier(), "x");
}

TEST(ParserTest, LetStatement_ErrorInExpression) {
    // setup
    std::string code("let x 42"); // should produce an error
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto error = std::get<ParserError>(parser_result);

    EXPECT_EQ(error.code, ErrorCode::SYNTAX_ERROR_EXPECTED_OPERATOR);
}

TEST(ParserTest, LetStatement_Expression) {
    // setup
    std::string code("let x = 5 + 5");
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto program_node = std::get<ASTBaseNode*>(parser_result);
    ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

    EXPECT_EQ(result->readStatements().size(), 1);
    EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::LET);
    auto letNode = static_cast<const ASTLetNode*>(result->readStatements()[0]);
    EXPECT_EQ(letNode->readExpression()->readType(), ASTNodeType::BINARY_EXPRESSION);
    EXPECT_EQ(letNode->readIdentifier(), "x");

    auto binaryExpression = static_cast<const ASTBinaryExpressionNode*>(letNode->readExpression());
    EXPECT_EQ(binaryExpression->readOperator(), OperatorType::ADDITION);
}

TEST(ParserTest, IncStatement) {
    // setup
    std::string code(R"(let x = 5
						x++)");
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto program_node = std::get<ASTBaseNode*>(parser_result);
    ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

    EXPECT_EQ(result->readStatements().size(), 2);
    EXPECT_EQ(result->readStatements()[1]->readType(), ASTNodeType::IDENTIFIER);
    auto identifier = static_cast<const ASTIdentifierNode*>(result->readStatements()[1]);
    EXPECT_EQ(identifier->readOperator()->readType(), ASTNodeType::INC_DEC_EXPRESSION);
    auto incDecNode = static_cast<const ASTIncDecNode*>(identifier->readOperator());
    EXPECT_EQ(incDecNode->readIsIncrement(), true);
}

TEST(ParserTest, EqualsOperator) {
    // setup
    std::string code("x == x");
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto program_node = std::get<ASTBaseNode*>(parser_result);
    ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

    EXPECT_EQ(result->readStatements().size(), 1);
    EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::COMPARISON_EXPRESSION);
    auto compare_expression = static_cast<const ASTComparisonExpressionNode*>(result->readStatements()[0]);
    EXPECT_EQ(compare_expression->readOperator(), OperatorType::EQUAL);
}

TEST(ParserTest, EqualsOperator_NotLegal) {
    // setup
    std::string code("x == x == x");
    Parser parser(code);

    // do - we know the node type returned is a program
    // auto parser_result = parser.parse();
    // auto program_node = std::get<ASTBaseNode*>(parser_result);
    // ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_TRUE(false) << "This test should fail and produce a error message";
}

TEST(ParserTest, WhileStatement_Legal) {
    // setup
    std::string code(R"(let i = 0
						while (i < 5) {
							i++
						}
						return i 		)");
    Parser parser(code);

    // do - we know the node type returned is a program
    auto parser_result = parser.parse();
    auto program_node = std::get<ASTBaseNode*>(parser_result);
    ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

    // validate
    EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

    EXPECT_EQ(result->readStatements().size(), 4);
    EXPECT_EQ(result->readStatements()[1]->readType(), ASTNodeType::WHILE);
    auto compare_expression = static_cast<const ASTComparisonExpressionNode*>(result->readStatements()[0]);
    EXPECT_EQ(compare_expression->readOperator(), OperatorType::EQUAL);
}