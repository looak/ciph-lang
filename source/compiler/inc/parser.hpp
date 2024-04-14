#pragma once

#include <string>

#include "lexar.hpp"

class ASTBaseNode;
class ASTExpressionNode;
class ASTIdentifierNode;
class ASTLetNode;
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
	ASTLetNode* parseLetStatement();
	ASTReturnNode* parseReturnStatement();
	ASTExpressionNode* parseAddativeExpression();
	ASTExpressionNode* parseMultiplicativeExpression();
	ASTIdentifierNode* parseIdentifier();
	ASTExpressionNode* parsePrimaryExpression();

	Lexar m_lexar;
};