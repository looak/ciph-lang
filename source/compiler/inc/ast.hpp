#pragma once

#include <string>
#include <stack>
#include <vector>

#include "lexar_defines.hpp"

namespace ciph {

enum class ASTNodeType
{
	PROGRAM,
	// literals
	NUMERIC_LITERAL,
	IDENTIFIER,

	// keywords
	LET,
	RETURN,
	WHILE,
	FUNCTION,

	/*
	STRING,
	CHARACTER,
	BOOLEAN,*/

	// scope and operators
	BINARY_EXPRESSION,
	INC_DEC_EXPRESSION,
	COMPARISON_EXPRESSION,
	
	CALL_EXPRESSION,

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

class ASTScopeNode : public ASTBaseNode
{
public:
	explicit ASTScopeNode(ASTNodeType type) : ASTBaseNode(type) {}
	virtual ~ASTScopeNode() override
	{
		for (auto& statement : m_statements)
		{
			delete statement;
		}
	}
	void addStatement(ASTBaseNode* statement) { m_statements.push_back(statement); }
	const std::vector<ASTBaseNode*>& readStatements() const { return m_statements; }

private:
	std::vector<ASTBaseNode*> m_statements;
};

class ASTProgramNode : public ASTScopeNode
{
public:
	ASTProgramNode() : ASTScopeNode(ASTNodeType::PROGRAM) {}
	~ASTProgramNode() final = default;
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

class ASTComparisonExpressionNode : public ASTExpressionNode
{
public:
	ASTComparisonExpressionNode(ASTBaseNode* lhs, ASTBaseNode* rhs, OperatorType op) :
		ASTExpressionNode(ASTNodeType::COMPARISON_EXPRESSION),
		m_left(lhs),
		m_right(rhs),
		m_operator(op)
	{}
	~ASTComparisonExpressionNode() override = default;

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

class ASTWhileNode : public ASTScopeNode {
public:
    explicit ASTWhileNode(ASTComparisonExpressionNode* condition)
        : ASTScopeNode(ASTNodeType::WHILE)
        , m_condition(condition) {}
    ~ASTWhileNode() final = default;

    [[nodiscard]] const ASTComparisonExpressionNode*
    readCondition() const {
        return m_condition;
    }

private:
    ASTComparisonExpressionNode* m_condition;
};

class ASTIncDecNode : public ASTExpressionNode
{
public:
	explicit ASTIncDecNode(bool isIncrement) : ASTExpressionNode(ASTNodeType::INC_DEC_EXPRESSION), m_isIncrement(isIncrement) {}
	~ASTIncDecNode() override = default;

	[[nodiscard]] bool readIsIncrement() const { return m_isIncrement; }

private:
	bool m_isIncrement;
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
	ASTIdentifierNode(const std::string& name, const ASTExpressionNode* op) : ASTExpressionNode(ASTNodeType::IDENTIFIER), m_name(name), m_operator(op) {}
	~ASTIdentifierNode() override = default;

	[[nodiscard]] const std::string& readName() const { return m_name; }	
	[[nodiscard]] const ASTExpressionNode* readOperator() const { return m_operator; }

private:
	std::string m_name;
	const ASTExpressionNode* m_operator;
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
	ASTLetNode(const std::string& identifier, ASTExpressionNode* expression) 
		: ASTBaseNode(ASTNodeType::LET)
		, m_identifier(identifier)
		, m_expression(expression) 
		{}
	~ASTLetNode() override = default;

	[[nodiscard]] const std::string& readIdentifier() const { return m_identifier; }
	[[nodiscard]] const ASTExpressionNode* readExpression() const { return m_expression; }

private:
	std::string m_identifier;
	ASTExpressionNode* m_expression;
};

class ASTFunctionNode : public ASTScopeNode {
private:
	std::string m_name;
public:
	ASTFunctionNode(std::string name)
		: ASTScopeNode(ASTNodeType::FUNCTION)
		, m_name(std::move(name))
		{}
	~ASTFunctionNode() final = default;

	[[nodiscard]] const std::string&
	readName() const {
		return m_name;
	}
};

class ASTCallNode : public ASTExpressionNode {
	private:
		std::string m_functionName;
	public:
		ASTCallNode(const std::string& functionName)
			: ASTExpressionNode(ASTNodeType::CALL_EXPRESSION)
			, m_functionName(functionName)
			{}
		~ASTCallNode() override = default;

		[[nodiscard]] const std::string&
		readFunctionName() const {
			return m_functionName;
		}

};

} // namespace ciph	