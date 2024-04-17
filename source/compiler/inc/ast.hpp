#pragma once

#include <string>
#include <stack>

#include "lexar_defines.hpp"

enum class ASTNodeType
{
	PROGRAM,
	// literals
	NUMERIC_LITERAL,     
	IDENTIFIER,

	// keywords
	LET,
	RETURN,

	/*
	STRING,
	CHARACTER,
	BOOLEAN,*/

	// scope and operators
	BINARY_EXPRESSION,

	END_OF_FILE,
	UNKNOWN
};

class ASTBaseNode
{
public:
	explicit ASTBaseNode(ASTNodeType type) : m_type(type) {}
	virtual ~ASTBaseNode() = default;
	ASTNodeType readType() const { return m_type; }


private:
	ASTNodeType m_type;
};


class ASTProgramNode : public ASTBaseNode
{
public:
	ASTProgramNode() : ASTBaseNode(ASTNodeType::PROGRAM) {}
	~ASTProgramNode() final = default;

	void addStatement(ASTBaseNode* statement) { m_statements.push_back(statement); }
	const std::vector<ASTBaseNode*>& readStatements() const { return m_statements; }

private:
	
	std::vector<ASTBaseNode*> m_statements;
};

class ASTExpressionNode : public ASTBaseNode
{
public:
	explicit ASTExpressionNode(ASTNodeType type) : ASTBaseNode(type) {}
	~ASTExpressionNode() override = default;
};

class ASTBinaryExpressionNode : public ASTExpressionNode
{
public:
	ASTBinaryExpressionNode(ASTBaseNode* lhs, ASTBaseNode* rhs, OperatorType op) :
		ASTExpressionNode(ASTNodeType::BINARY_EXPRESSION),
		m_left(lhs),
		m_right(rhs),
		m_operator(op)
	{}
	~ASTBinaryExpressionNode() override = default;

	[[nodiscard]] ASTBaseNode* editLeft() { return m_left; }
	[[nodiscard]] ASTBaseNode* editRight() { return m_right; }
	[[nodiscard]] OperatorType& editOperator() { return m_operator; }

	[[nodiscard]] const ASTBaseNode* readLeft() const { return m_left; }
	[[nodiscard]] const ASTBaseNode* readRight() const { return m_right; }
	[[nodiscard]] OperatorType readOperator() const { return m_operator; }

private:
	ASTBaseNode* m_left;
	ASTBaseNode* m_right;
	OperatorType m_operator;
};

class ASTNumericLiteralNode : public ASTExpressionNode
{
public:
	explicit ASTNumericLiteralNode(int16_t value) : ASTExpressionNode(ASTNodeType::NUMERIC_LITERAL), m_value(value) {}
	~ASTNumericLiteralNode() override = default;

	[[nodiscard]] int16_t readValue() const { return m_value; }

private:
	int16_t m_value;
};

class ASTIdentifierNode : public ASTExpressionNode
{
public:
	explicit ASTIdentifierNode(const std::string& name) : ASTExpressionNode(ASTNodeType::IDENTIFIER), m_name(name) {}
	~ASTIdentifierNode() override = default;

	[[nodiscard]] const std::string& readName() const { return m_name; }
private:
	std::string m_name;
};

class ASTReturnNode : public ASTBaseNode
{
public:
	explicit ASTReturnNode(ASTBaseNode* expression) : ASTBaseNode(ASTNodeType::RETURN), m_expression(expression) {}
	~ASTReturnNode() override = default;

	[[nodiscard]] ASTBaseNode* editExpression() { return m_expression; }
	[[nodiscard]] const ASTBaseNode* readExpression() const { return m_expression; }

private:
	ASTBaseNode* m_expression;
};

class ASTLetNode : public ASTBaseNode
{
public:
	ASTLetNode(const std::string& identifier, ASTExpressionNode* expression) : ASTBaseNode(ASTNodeType::LET), m_identifier(identifier), m_expression(expression) {}
	~ASTLetNode() override = default;

	[[nodiscard]] const std::string& readIdentifier() const { return m_identifier; }
	[[nodiscard]] const ASTExpressionNode* readExpression() const { return m_expression; }

private:
	std::string m_identifier;
	ASTExpressionNode* m_expression;
};