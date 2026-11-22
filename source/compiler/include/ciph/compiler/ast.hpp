#pragma once

#include <cstdint>
#include <string>
#include <stack>
#include <vector>

#include <ciph/compiler/lexar_defines.hpp>

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

// The tokens an AST node was built from: indices into the lexer's token list
// (Lexar::readTokens()), both ends inclusive. A token knows its line, column
// and length, so a range leads back to the exact source text.
//
// A node that was not built from source text, such as one a test creates by
// hand, has no range: pass SourceRange{}.
struct SourceRange
{
	static constexpr uint32_t no_token = 0xFFFFFFFF;

	uint32_t first = no_token;
	uint32_t last = no_token;

	// The token that best stands for the node: a let's or fn's name, a
	// binary expression's operator. Errors point here. Unset means `first`.
	uint32_t focus = no_token;

	[[nodiscard]] bool hasTokens() const { return first != no_token; }
	[[nodiscard]] uint32_t focusToken() const { return focus != no_token ? focus : first; }
};

class ASTBaseNode
{
public:
	ASTBaseNode(ASTNodeType type, SourceRange range) : m_type(type), m_range(range) {}
	virtual ~ASTBaseNode() = default;
	ASTNodeType readType() const { return m_type; }
	[[nodiscard]] SourceRange readSourceRange() const { return m_range; }

protected:
	void setSourceRange(SourceRange range) { m_range = range; }

private:
	ASTNodeType m_type;
	SourceRange m_range;
};

class ASTScopeNode : public ASTBaseNode
{
public:
	ASTScopeNode(ASTNodeType type, SourceRange range) : ASTBaseNode(type, range) {}
	virtual ~ASTScopeNode() override
	{
		for (auto& statement : m_statements)
		{
			delete statement;
		}
	}
	void addStatement(ASTBaseNode* statement) { m_statements.push_back(statement); }
	const std::vector<ASTBaseNode*>& readStatements() const { return m_statements; }

	// A scope node is created before its statements are parsed, when only its
	// first token is known. The parser calls this once it reaches the end.
	void closeSourceRange(uint32_t lastToken) { setSourceRange({readSourceRange().first, lastToken}); }

private:
	std::vector<ASTBaseNode*> m_statements;
};

class ASTProgramNode : public ASTScopeNode
{
public:
	explicit ASTProgramNode(SourceRange range) : ASTScopeNode(ASTNodeType::PROGRAM, range) {}
	~ASTProgramNode() final = default;
};

class ASTExpressionNode : public ASTBaseNode
{
public:
	ASTExpressionNode(ASTNodeType type, SourceRange range) : ASTBaseNode(type, range) {}
	~ASTExpressionNode() override = default;
};

class ASTBinaryExpressionNode : public ASTExpressionNode
{
public:
	ASTBinaryExpressionNode(SourceRange range, ASTBaseNode* lhs, ASTBaseNode* rhs, OperatorType op) :
		ASTExpressionNode(ASTNodeType::BINARY_EXPRESSION, range),
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
	ASTComparisonExpressionNode(SourceRange range, ASTBaseNode* lhs, ASTBaseNode* rhs, OperatorType op) :
		ASTExpressionNode(ASTNodeType::COMPARISON_EXPRESSION, range),
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
    ASTWhileNode(SourceRange range, ASTComparisonExpressionNode* condition)
        : ASTScopeNode(ASTNodeType::WHILE, range)
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
	ASTIncDecNode(SourceRange range, bool isIncrement) : ASTExpressionNode(ASTNodeType::INC_DEC_EXPRESSION, range), m_isIncrement(isIncrement) {}
	~ASTIncDecNode() override = default;

	[[nodiscard]] bool readIsIncrement() const { return m_isIncrement; }

private:
	bool m_isIncrement;
};

class ASTNumericLiteralNode : public ASTExpressionNode
{
public:
	ASTNumericLiteralNode(SourceRange range, int16_t value) : ASTExpressionNode(ASTNodeType::NUMERIC_LITERAL, range), m_value(value) {}
	~ASTNumericLiteralNode() override = default;

	[[nodiscard]] int16_t readValue() const { return m_value; }

private:
	int16_t m_value;
};

class ASTIdentifierNode : public ASTExpressionNode
{
public:
	ASTIdentifierNode(SourceRange range, const std::string& name, const ASTExpressionNode* op) : ASTExpressionNode(ASTNodeType::IDENTIFIER, range), m_name(name), m_operator(op) {}
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
	ASTReturnNode(SourceRange range, ASTBaseNode* expression) : ASTBaseNode(ASTNodeType::RETURN, range), m_expression(expression) {}
	~ASTReturnNode() override = default;

	[[nodiscard]] ASTBaseNode* editExpression() { return m_expression; }
	[[nodiscard]] const ASTBaseNode* readExpression() const { return m_expression; }

private:
	ASTBaseNode* m_expression;
};

class ASTLetNode : public ASTBaseNode
{
public:
	ASTLetNode(SourceRange range, const std::string& identifier, ASTExpressionNode* expression)
		: ASTBaseNode(ASTNodeType::LET, range)
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
	ASTFunctionNode(SourceRange range, std::string name)
		: ASTScopeNode(ASTNodeType::FUNCTION, range)
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
		ASTCallNode(SourceRange range, const std::string& functionName)
			: ASTExpressionNode(ASTNodeType::CALL_EXPRESSION, range)
			, m_functionName(functionName)
			{}
		~ASTCallNode() override = default;

		[[nodiscard]] const std::string&
		readFunctionName() const {
			return m_functionName;
		}

};

} // namespace ciph
