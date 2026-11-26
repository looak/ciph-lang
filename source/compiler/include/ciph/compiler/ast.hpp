#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <stack>
#include <vector>

#include <ciph/compiler/bit_field.hpp>
#include <ciph/compiler/diagnostics.hpp>
#include <ciph/compiler/lexer_defines.hpp>

namespace ciph {

class Token;

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
	COMPARISON_EXPRESSION,
	UNARY_EXPRESSION,

	CALL_EXPRESSION,

	END_OF_FILE,
	UNKNOWN
};

// The tokens an AST node was built from: indices into the lexer's token list
// (Lexer::readTokens()), both ends inclusive. A token knows its line, column
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

class ASTBaseNode;

// Where `node` came from in the source, for a diagnostic. A node with a focus
// token gives that token's span; one without gives its first token's start to
// its last token's end. A node with no range gives an empty location.
//
// `tokens` is the list the node's range indexes into (Parser::readTokens()).
[[nodiscard]] Location locationOf(const ASTBaseNode& node, std::span<const Token> tokens);

// Where a node came from. `Graph` joins when the graph frontend builds AST.
enum class Origin : uint8_t
{
	Text,        // parsed from source text
	Synthesized, // made up by the compiler, such as the 0 behind `return;`
};

class ASTBaseNode
{
public:
	// Every node carries 64 bits of packed fields. Bits below kBaseBitsEnd
	// mean the same on every node; a subclass packs its own fields from
	// kBaseBitsEnd up, so the same upper bits mean different things on
	// different node types.
	using OriginField = BitField<Origin, 0, 2>;
	using HasSideEffectField = OriginField::Next<bool, 1>;
	static constexpr unsigned kBaseBitsEnd = 12;
	static_assert(HasSideEffectField::kEnd <= kBaseBitsEnd, "the base node's fields have outgrown their 12 bits");

	ASTBaseNode(ASTNodeType type, SourceRange range) : m_type(type), m_range(range) {}
	virtual ~ASTBaseNode() = default;
	ASTNodeType readType() const { return m_type; }
	[[nodiscard]] SourceRange readSourceRange() const { return m_range; }

	[[nodiscard]] Origin readOrigin() const { return readField<OriginField>(); }
	void setOrigin(Origin origin) { writeField<OriginField>(origin); }

	// True when evaluating this node itself changes state: a call, `x++`.
	// It says nothing about the children, so for `x++ + 1` the `+` is false.
	[[nodiscard]] bool readHasSideEffect() const { return readField<HasSideEffectField>(); }

protected:
	void setSourceRange(SourceRange range) { m_range = range; }
	void setHasSideEffect(bool hasSideEffect) { writeField<HasSideEffectField>(hasSideEffect); }

	template <class Field>
	[[nodiscard]] typename Field::Type readField() const { return Field::get(m_bits); }
	template <class Field>
	void writeField(typename Field::Type value) { m_bits = Field::set(m_bits, value); }

private:
	ASTNodeType m_type;
	SourceRange m_range;
	uint64_t m_bits = 0;
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
	// Only `last` changes, so a focus set at creation survives.
	void closeSourceRange(uint32_t lastToken)
	{
		SourceRange range = readSourceRange();
		range.last = lastToken;
		setSourceRange(range);
	}

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

// An operator written before its one operand: `-x`. The operator is the
// node's first token, so errors point at it without a focus being set.
class ASTUnaryExpressionNode : public ASTExpressionNode
{
public:
	ASTUnaryExpressionNode(SourceRange range, ASTBaseNode* operand, OperatorType op) :
		ASTExpressionNode(ASTNodeType::UNARY_EXPRESSION, range),
		m_operand(operand),
		m_operator(op)
	{}
	~ASTUnaryExpressionNode() override = default;

	[[nodiscard]] const ASTBaseNode* readOperand() const { return m_operand; }
	[[nodiscard]] OperatorType readOperator() const { return m_operator; }

private:
	ASTBaseNode* m_operand;
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

class ASTNumericLiteralNode : public ASTExpressionNode
{
public:
	ASTNumericLiteralNode(SourceRange range, int16_t value) : ASTExpressionNode(ASTNodeType::NUMERIC_LITERAL, range), m_value(value) {}
	~ASTNumericLiteralNode() override = default;

	[[nodiscard]] int16_t readValue() const { return m_value; }

private:
	int16_t m_value;
};

// `++` or `--` written after a variable. Three bits leave room for the
// prefix forms.
enum class IncDec : uint8_t
{
	None,
	PostIncrement,
	PostDecrement,
};

class ASTIdentifierNode : public ASTExpressionNode
{
public:
	using IncDecField = BitField<IncDec, kBaseBitsEnd, 3>;

	ASTIdentifierNode(SourceRange range, std::string_view name, IncDec incDec = IncDec::None)
		: ASTExpressionNode(ASTNodeType::IDENTIFIER, range)
		, m_name(name)
	{
		writeField<IncDecField>(incDec);
		setHasSideEffect(incDec != IncDec::None);
	}
	~ASTIdentifierNode() override = default;

	[[nodiscard]] std::string_view readName() const { return m_name; }
	[[nodiscard]] IncDec readIncDec() const { return readField<IncDecField>(); }

private:
	std::string_view m_name;
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
	ASTLetNode(SourceRange range, std::string_view identifier, ASTExpressionNode* expression)
		: ASTBaseNode(ASTNodeType::LET, range)
		, m_identifier(identifier)
		, m_expression(expression)
		{}
	~ASTLetNode() override = default;

	[[nodiscard]] std::string_view readIdentifier() const { return m_identifier; }
	[[nodiscard]] const ASTExpressionNode* readExpression() const { return m_expression; }

private:
	std::string_view m_identifier;
	ASTExpressionNode* m_expression;
};

class ASTFunctionNode : public ASTScopeNode {
private:
	std::string_view m_name;
public:
	ASTFunctionNode(SourceRange range, std::string_view name)
		: ASTScopeNode(ASTNodeType::FUNCTION, range)
		, m_name(name)
		{}
	~ASTFunctionNode() final = default;

	[[nodiscard]] std::string_view
	readName() const {
		return m_name;
	}
};

class ASTCallNode : public ASTExpressionNode {
	private:
		std::string_view m_functionName;
	public:
		ASTCallNode(SourceRange range, std::string_view functionName)
			: ASTExpressionNode(ASTNodeType::CALL_EXPRESSION, range)
			, m_functionName(functionName)
		{
			setHasSideEffect(true); // calling a function may change state
		}
		~ASTCallNode() override = default;

		[[nodiscard]] std::string_view
		readFunctionName() const {
			return m_functionName;
		}

};

} // namespace ciph
