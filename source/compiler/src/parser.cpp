#include "parser.hpp"
#include "ast.hpp"

#include <fmt/core.h>

Parser::Parser(const std::string& input) : m_lexar(input)
{
}

ASTBaseNode* Parser::parse()
{
	return parseProgram();
}

ASTProgramNode*
Parser::parseProgram()
{
	ASTProgramNode* program = new ASTProgramNode();
	auto token = m_lexar.peek();

	while (token.readType() != TokenType::END_OF_FILE)
	{
		auto stmnt = parseStatement();
		if (stmnt != nullptr)
			program->addStatement(stmnt);
		token = m_lexar.peek();
	}
	return program;
}

ASTBaseNode*
Parser::parseStatement()
{
	auto token = m_lexar.peek();
	switch (token.readType())
	{
		case TokenType::RETURN:
			return parseReturnStatement();
		case TokenType::LET:
			return parseLetStatement();
		default:
			return parseAddativeExpression();
	}
}

ASTReturnNode*
Parser::parseReturnStatement()
{	
	m_lexar.pop();
	auto token = m_lexar.peek();
	ASTExpressionNode* expression = nullptr;
	if (token.readType() == TokenType::END_OF_FILE)
	{
		// return 0 as default;
		expression = new ASTNumericLiteralNode(0);
	}
	else
	{
		expression = parseAddativeExpression();
	}

	return new ASTReturnNode(expression);
}

ASTExpressionNode*
Parser::parseAddativeExpression()
{
	ASTExpressionNode* left = parseMultiplicativeExpression();
	ASTExpressionNode* right = nullptr;

	auto op = m_lexar.peek();

	if (op.readOperator() == OperatorType::ADDITION || op.readOperator() == OperatorType::SUBTRACTION)
	{
		m_lexar.pop();
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

	auto op = m_lexar.peek();
	
	if (op.readOperator() == OperatorType::MULTIPLICATION || op.readOperator() == OperatorType::DIVISION)
	{
		m_lexar.pop();
		right = parsePrimaryExpression();
		return new ASTBinaryExpressionNode(left, right, op.readOperator());
	}

	return left;
}

ASTExpressionNode* 
Parser::parsePrimaryExpression()
{
    Token token = m_lexar.peek();

	switch (token.readType())
	{
		case TokenType::NUMBER:
            m_lexar.pop();
			return new ASTNumericLiteralNode(std::stoi(token.readValue()));
		case TokenType::IDENTIFIER:
			m_lexar.pop();
			return new ASTIdentifierNode(token.readValue());
		case TokenType::OPEN_PAREN:
		{
			m_lexar.pop();
			ASTExpressionNode* expression = parseAddativeExpression();
			if (m_lexar.expect(TokenType::CLOSE_PAREN) == false)
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

ASTLetNode*
Parser::parseLetStatement()
{
	m_lexar.pop();
	auto token = m_lexar.peek();
	if (token.readType() != TokenType::IDENTIFIER)
	{
		fmt::print(stderr, "Expected identifier\n");
		return nullptr;
	}

	std::string name = token.readValue();
	m_lexar.pop();

	if (m_lexar.popOperator(OperatorType::ASSIGNMENT) == false)
	{
		fmt::print(stderr, "Expected assignment operator\n");
		return nullptr;
	}

	ASTExpressionNode* expression = parseAddativeExpression();
	return new ASTLetNode(name, expression);
}

ASTIdentifierNode*
Parser::parseIdentifier()
{
	auto token = m_lexar.pop();
	if (token.readType() != TokenType::IDENTIFIER)
	{
		fmt::print(stderr, "Expected identifier\n");
		return nullptr;
	}

	return new ASTIdentifierNode(token.readValue());
}