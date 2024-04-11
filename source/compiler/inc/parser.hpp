#pragma once

#include <string>

#include "lexar.hpp"

class ASTBaseNode;
class ASTExpressionNode;

class Parser
{
public:
	explicit Parser(std::string& input);
	~Parser() = default;

	ASTBaseNode* parse();


private:
	ASTExpressionNode* parseAddativeExpression();
	ASTExpressionNode* parseMultiplicativeExpression();

	ASTExpressionNode* parseBinaryOperationExpression();	

	ASTExpressionNode* parsePrimaryExpression();

	Lexar m_lexar;


};