#include <gtest/gtest.h>

#include "parser.hpp"
#include "ast.hpp"

TEST(ParserTest, SimpleAddExpression)
{
	// setup
	std::string code("1 + 2");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	ASSERT_EQ(result->readStatements()[0]->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto binaryExpression = static_cast<ASTBinaryExpressionNode*>(result->readStatements()[0]);
	EXPECT_EQ(binaryExpression->readOperator() , OperatorType::ADDITION);
}

TEST(ParserTest, OrderOfOperations_MulBeforeAdd)
{
	// setup
	std::string code("2 * 2 + 3");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto addExpression = static_cast<const ASTBinaryExpressionNode*>(result->readStatements()[0]);
	EXPECT_EQ(addExpression->readOperator() , OperatorType::ADDITION);

	auto left = addExpression->readLeft();
	EXPECT_EQ(left->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto mulExpression = static_cast<const ASTBinaryExpressionNode*>(left);
	EXPECT_EQ(mulExpression->readOperator() , OperatorType::MULTIPLICATION);
}

TEST(ParserTest, OrderOfOperations_ParenthaseseBeforeMul)
{
	// setup
	std::string code("2 * (2 + 3)");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto mulExpression = static_cast<const ASTBinaryExpressionNode*>(result->readStatements()[0]);
	EXPECT_EQ(mulExpression->readOperator() , OperatorType::MULTIPLICATION);

	auto right = mulExpression->readRight();
	EXPECT_EQ(right->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto addExpression = static_cast<const ASTBinaryExpressionNode*>(right);
	EXPECT_EQ(addExpression->readOperator() , OperatorType::ADDITION);
}

TEST(ParserTest, ReturnStatement_DefaultNumericNode_ValueZero)
{
	// setup
	std::string code("return");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);	
	auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);
	
	EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::NUMERIC_LITERAL);
	auto numericNode = static_cast<const ASTNumericLiteralNode*>(returnNode->readExpression());
	EXPECT_EQ(numericNode->readValue(), 0);
}

TEST(ParserTest, ReturnStatement_NumericNodeGivenValue)
{
	// setup
	std::string code("return 5");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);	
	auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);
	
	EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::NUMERIC_LITERAL);
	auto numericNode = static_cast<const ASTNumericLiteralNode*>(returnNode->readExpression());
	EXPECT_EQ(numericNode->readValue(), 5);
}

TEST(ParserTest, ReturnStatement_NumericNodeExpression)
{
	// setup
	std::string code("return 5 + 2");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);	
	auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);
	
	EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::BINARY_EXPRESSION);	
}

TEST(ParserTest, LetStatement)
{
	// setup
	std::string code("let x = 5");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::LET);
	auto letNode = static_cast<const ASTLetNode*>(result->readStatements()[0]);
	EXPECT_EQ(letNode->readExpression()->readType(), ASTNodeType::NUMERIC_LITERAL);
	EXPECT_EQ(letNode->readIdentifier(), "x");
}

TEST(ParserTest, LetStatement_Expression)
{
	// setup
	std::string code("let x = 5 + 5");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

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

TEST(ParserTest, IncStatement)
{
	// setup
	std::string code(R"(let x = 5
						x++)");
	Parser parser(code);

	// do - we know the node type returned is a program	
	ASTProgramNode* result = static_cast<ASTProgramNode*>(parser.parse());

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 2);
	EXPECT_EQ(result->readStatements()[1]->readType(), ASTNodeType::IDENTIFIER);
	auto identifier = static_cast<const ASTIdentifierNode*>(result->readStatements()[1]);	
	EXPECT_EQ(identifier->readOperator()->readType(), ASTNodeType::INC_DEC_EXPRESSION);
	auto incDecNode = static_cast<const ASTIncDecNode*>(identifier->readOperator());
	EXPECT_EQ(incDecNode->readIsIncrement(), true);
}