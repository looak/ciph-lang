#include "parser.hpp"
#include "ast.hpp"

#include <fmt/core.h>

Parser::Parser(std::string& input) : m_lexar(input)
{
}

ASTBaseNode* Parser::parse()
{
	ASTProgramNode* node = new ASTProgramNode();
	auto token = m_lexar.peekNextToken();

	while (token.readType() != TokenType::END_OF_FILE)
	{
		ASTExpressionNode* expression = parseAddativeExpression();
		if (expression != nullptr)
			node->addStatement(expression);
		token = m_lexar.popNextToken();
	}

	return node;
}

ASTExpressionNode*
Parser::parseAddativeExpression()
{
	ASTExpressionNode* left = parseMultiplicativeExpression();
	ASTExpressionNode* right = nullptr;

	auto op = m_lexar.peekNextToken();

	if (op.readOperator() == OperatorType::ADDITION || op.readOperator() == OperatorType::SUBTRACTION)
	{
		m_lexar.popNextToken();
		right = parseMultiplicativeExpression();		
		return new ASTBinaryExpressionNode(left, right, op.readOperator());
	}

	return left;
}

ASTExpressionNode*
Parser::parseMultiplicativeExpression()
{
	ASTExpressionNode* left = parsePrimaryExpression();
	ASTExpressionNode* right = nullptr;

	auto op = m_lexar.peekNextToken();
	
	if (op.readOperator() == OperatorType::MULTIPLICATION || op.readOperator() == OperatorType::DIVISION)
	{
		m_lexar.popNextToken();
		right = parsePrimaryExpression();
		return new ASTBinaryExpressionNode(left, right, op.readOperator());
	}

	return left;
}

ASTExpressionNode* 
Parser::parsePrimaryExpression()
{
    Token token = m_lexar.peekNextToken();

	switch (token.readType())
	{
		case TokenType::NUMBER:
            m_lexar.popNextToken();
			return new ASTNumericLiteralNode(std::stoi(token.readValue()));
		case TokenType::IDENTIFIER:
			return new ASTIdentifierNode(token.readValue());
		case TokenType::OPEN_PAREN:
		{
			m_lexar.popNextToken();
			ASTExpressionNode* expression = parseAddativeExpression();
			if (m_lexar.expectNextToken(TokenType::CLOSE_PAREN) == false)
			{
				fmt::print("Expected closing parenthesis\n");
				return nullptr;	
			}
			return expression;
		}
		default:
			return nullptr;
	}

}