#include "parser.hpp"
#include "ast.hpp"

Parser::Parser(std::string& input) : m_lexar(input)
{
}

ASTBaseNode* Parser::parse()
{
	ASTProgramNode* node = new ASTProgramNode();
	auto token = m_lexar.peekNextToken();

	while (token.readType() != TokenType::END_OF_FILE)
	{
		ASTExpressionNode* expression = parseBinaryOperationExpression();
		if (expression != nullptr)
			node->addStatement(expression);
		token = m_lexar.popNextToken();
	}

	return node;
}

//ASTExpressionNode*

ASTExpressionNode*
Parser::parseBinaryOperationExpression()
{
	ASTExpressionNode* left = parseBinaryOperationExpression();
	ASTExpressionNode* right = nullptr;

	auto op = m_lexar.peekNextToken();
	
	if (op.readOperator() == OperatorType::MULTIPLICATION || op.readOperator() == OperatorType::DIVISION)
		right = parseBinaryOperationExpression();

	return new ASTBinaryExpressionNode(left, right);
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
		/*case TokenType::VAR:
			return new ASTExpressionNode(ASTNodeType::VAR);*/
		default:
			return nullptr;
	}

}