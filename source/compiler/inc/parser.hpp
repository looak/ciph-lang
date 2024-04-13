#pragma once

#include <string>

#include "lexar.hpp"

class ASTBaseNode;
class ASTExpressionNode;
class ASTProgramNode;
class ASTReturnNode;

class Parser
{
public:
	explicit Parser(const std::string& input);
	~Parser() = default;

	ASTBaseNode* parse();
	ASTProgramNode* parseProgram();


private:
	ASTBaseNode* parseStatement();
	ASTReturnNode* parseReturnStatement();
	ASTExpressionNode* parseAddativeExpression();
	ASTExpressionNode* parseMultiplicativeExpression();

	// ASTExpressionNode* parseBinaryOperationExpression();	

	ASTExpressionNode* parsePrimaryExpression();

	Lexar m_lexar;
};