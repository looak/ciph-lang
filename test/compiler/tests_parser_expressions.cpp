// Parser tests for arithmetic expressions: which operator binds tighter, and
// how a chain of operators at the same level is grouped.

#include <string>

#include <fmt/core.h>
#include <gtest/gtest.h>

#include <ciph/compiler/ast.hpp>
#include "text_parser.hpp"

using namespace ciph;

namespace {

// Writes an expression back out with every binary node in parentheses, so a
// test can state the shape of the tree as text: "((1 + 2) + 3)".
std::string
shapeOf(const ASTBaseNode* node) {
	if (node == nullptr) {
		return "<null>";
	}
	switch (node->readType()) {
		case ASTNodeType::NUMERIC_LITERAL:
			return fmt::format("{}", static_cast<const ASTNumericLiteralNode*>(node)->readValue());
		case ASTNodeType::IDENTIFIER:
			return std::string(static_cast<const ASTIdentifierNode*>(node)->readName());
		case ASTNodeType::BINARY_EXPRESSION: {
			auto binary = static_cast<const ASTBinaryExpressionNode*>(node);
			const char* op = "?";
			switch (binary->readOperator()) {
				case OperatorType::ADDITION: op = "+"; break;
				case OperatorType::SUBTRACTION: op = "-"; break;
				case OperatorType::MULTIPLICATION: op = "*"; break;
				case OperatorType::DIVISION: op = "/"; break;
				default: break;
			}
			return fmt::format("({} {} {})", shapeOf(binary->readLeft()), op, shapeOf(binary->readRight()));
		}
		case ASTNodeType::COMPARISON_EXPRESSION: {
			auto comparison = static_cast<const ASTComparisonExpressionNode*>(node);
			const char* op = "?";
			switch (comparison->readOperator()) {
				case OperatorType::EQUAL: op = "=="; break;
				case OperatorType::NOT_EQUAL: op = "!="; break;
				case OperatorType::LESS_THAN: op = "<"; break;
				case OperatorType::GREATER_THAN: op = ">"; break;
				default: break;
			}
			return fmt::format("({} {} {})", shapeOf(comparison->readLeft()), op, shapeOf(comparison->readRight()));
		}
		case ASTNodeType::UNARY_EXPRESSION: {
			auto unary = static_cast<const ASTUnaryExpressionNode*>(node);
			const char* op = unary->readOperator() == OperatorType::SUBTRACTION ? "-" : "?";
			return fmt::format("({}{})", op, shapeOf(unary->readOperand()));
		}
		default:
			return "<other>";
	}
}

// The expression of the program's only statement, which has to be a return.
// An expression cannot stand alone as a statement, so the tests here parse
// `return <expression>;` and look at what is returned.
const ASTBaseNode*
returnedExpression(const ASTProgramNode* program) {
	if (program->readStatements().size() != 1 || program->readStatements()[0]->readType() != ASTNodeType::RETURN) {
		return nullptr;
	}
	return static_cast<const ASTReturnNode*>(program->readStatements()[0])->readExpression();
}

// Parses `code`, which has to be a single return statement written out in
// full, and returns the shape of what it returns. A parse error comes back as
// "<parse error>" so the test fails on the comparison instead of crashing.
std::string
returnedShapeOf(const std::string& code) {
	TextParser parser(code);
	auto parser_result = parser.parse();
	if (!parser_result.has_value()) {
		return "<parse error>";
	}
	return shapeOf(returnedExpression(static_cast<const ASTProgramNode*>(*parser_result)));
}

// Shorthand for the tests that only care about the expression: wraps it as
// `return <expression>;`.
std::string
parsedShapeOf(const std::string& expression) {
	return returnedShapeOf(fmt::format("return {};", expression));
}

} // namespace

// ---- Operator precedence ----

TEST(ParserExpressionTest, SimpleAdd) {
	// setup
	std::string code("return 1 + 2;");
	TextParser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto program_node = parser_result.value();
	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	auto expression = returnedExpression(result);
	ASSERT_NE(expression, nullptr);
	ASSERT_EQ(expression->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto binaryExpression = static_cast<const ASTBinaryExpressionNode*>(expression);
	EXPECT_EQ(binaryExpression->readOperator(), OperatorType::ADDITION);
}

TEST(ParserExpressionTest, OrderOfOperations_MulBeforeAdd) {
	// setup
	std::string code("return 2 * 2 + 3;");
	TextParser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto program_node = parser_result.value();
	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	auto expression = returnedExpression(result);
	ASSERT_NE(expression, nullptr);
	ASSERT_EQ(expression->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto addExpression = static_cast<const ASTBinaryExpressionNode*>(expression);
	EXPECT_EQ(addExpression->readOperator(), OperatorType::ADDITION);

	auto left = addExpression->readLeft();
	EXPECT_EQ(left->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto mulExpression = static_cast<const ASTBinaryExpressionNode*>(left);
	EXPECT_EQ(mulExpression->readOperator(), OperatorType::MULTIPLICATION);
}

TEST(ParserExpressionTest, OrderOfOperations_ParenthaseseBeforeMul) {
	// setup
	std::string code("return 2 * (2 + 3);");
	TextParser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto program_node = parser_result.value();
	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	auto expression = returnedExpression(result);
	ASSERT_NE(expression, nullptr);
	ASSERT_EQ(expression->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto mulExpression = static_cast<const ASTBinaryExpressionNode*>(expression);
	EXPECT_EQ(mulExpression->readOperator(), OperatorType::MULTIPLICATION);

	auto right = mulExpression->readRight();
	EXPECT_EQ(right->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto addExpression = static_cast<const ASTBinaryExpressionNode*>(right);
	EXPECT_EQ(addExpression->readOperator(), OperatorType::ADDITION);
}

// ---- Chained operators ----
//
// An operator can repeat at its own precedence level, and such a chain groups
// from the left: `10 - 4 - 3` is `(10 - 4) - 3`, which is 3, and not
// `10 - (4 - 3)`, which is 9.

TEST(ParserExpressionTest, Chained_Addition) {
	EXPECT_EQ(parsedShapeOf("1 + 2 + 3"), "((1 + 2) + 3)");
}

TEST(ParserExpressionTest, Chained_SubtractionGroupsFromTheLeft) {
	EXPECT_EQ(parsedShapeOf("10 - 4 - 3"), "((10 - 4) - 3)");
}

TEST(ParserExpressionTest, Chained_Multiplication) {
	EXPECT_EQ(parsedShapeOf("2 * 3 * 4"), "((2 * 3) * 4)");
}

TEST(ParserExpressionTest, Chained_DivisionGroupsFromTheLeft) {
	EXPECT_EQ(parsedShapeOf("24 / 4 / 2"), "((24 / 4) / 2)");
}

TEST(ParserExpressionTest, Chained_MixedAtOneLevel) {
	EXPECT_EQ(parsedShapeOf("1 + 2 - 3 + 4"), "(((1 + 2) - 3) + 4)");
	EXPECT_EQ(parsedShapeOf("8 / 2 * 3"), "((8 / 2) * 3)");
}

TEST(ParserExpressionTest, Chained_MultiplicationStillBindsTighter) {
	EXPECT_EQ(parsedShapeOf("1 + 2 * 3 + 4"), "((1 + (2 * 3)) + 4)");
	EXPECT_EQ(parsedShapeOf("1 * 2 + 3 * 4 * 5 - 6"), "(((1 * 2) + ((3 * 4) * 5)) - 6)");
}

TEST(ParserExpressionTest, Chained_InsideParentheses) {
	EXPECT_EQ(parsedShapeOf("(1 + 2 + 3) * 2"), "(((1 + 2) + 3) * 2)");
}

TEST(ParserExpressionTest, Chained_Variables) {
	EXPECT_EQ(parsedShapeOf("a + b + c"), "((a + b) + c)");
}

TEST(ParserExpressionTest, Chained_InLetAndReturn) {
	TextParser parser("let x = 1 + 2 + 3; return x - 1 - 1;");

	auto parser_result = parser.parse();
	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	ASSERT_EQ(program->readStatements().size(), 2u);

	auto let = static_cast<const ASTLetNode*>(program->readStatements()[0]);
	EXPECT_EQ(shapeOf(let->readExpression()), "((1 + 2) + 3)");
	auto ret = static_cast<const ASTReturnNode*>(program->readStatements()[1]);
	EXPECT_EQ(shapeOf(ret->readExpression()), "((x - 1) - 1)");
}

TEST(ParserExpressionTest, Chained_OnEitherSideOfAComparison) {
	TextParser parser("return 1 + 2 + 3 == 2 * 3 * 1;");

	auto parser_result = parser.parse();
	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	auto ret = static_cast<const ASTReturnNode*>(program->readStatements()[0]);
	ASSERT_EQ(ret->readExpression()->readType(), ASTNodeType::COMPARISON_EXPRESSION);
	auto comparison = static_cast<const ASTComparisonExpressionNode*>(ret->readExpression());

	EXPECT_EQ(shapeOf(comparison->readLeft()), "((1 + 2) + 3)");
	EXPECT_EQ(shapeOf(comparison->readRight()), "((2 * 3) * 1)");
}

TEST(ParserExpressionTest, Chained_SourceRangeCoversTheWholeChain) {
	// tokens: return(0) 1(1) +(2) 2(3) +(4) 3(5) ;(6)
	TextParser parser("return 1 + 2 + 3;");

	auto parser_result = parser.parse();
	ASSERT_TRUE(parser_result.has_value());
	auto expression = returnedExpression(static_cast<const ASTProgramNode*>(*parser_result));
	ASSERT_NE(expression, nullptr);
	ASSERT_EQ(expression->readType(), ASTNodeType::BINARY_EXPRESSION);
	auto outer = static_cast<const ASTBinaryExpressionNode*>(expression);

	EXPECT_EQ(outer->readSourceRange().first, 1u);
	EXPECT_EQ(outer->readSourceRange().last, 5u);
	// the inner node is `1 + 2`
	EXPECT_EQ(outer->readLeft()->readSourceRange().first, 1u);
	EXPECT_EQ(outer->readLeft()->readSourceRange().last, 3u);
}

// ---- Chains and parentheses in whole statements ----
//
// The same rules with the statement written out in full.

TEST(ParserExpressionTest, Return_ChainInParenthesesThenMultiplied) {
	EXPECT_EQ(returnedShapeOf("return (1 + 2 + 3) * 2;"), "(((1 + 2) + 3) * 2)");
}

TEST(ParserExpressionTest, Return_MultipliedByChainInParentheses) {
	EXPECT_EQ(returnedShapeOf("return 2 * (1 + 2 + 3);"), "(2 * ((1 + 2) + 3))");
}

TEST(ParserExpressionTest, Return_ChainOfParenthesisedGroups) {
	EXPECT_EQ(returnedShapeOf("return (1 + 2) * (3 + 4) * 5;"), "(((1 + 2) * (3 + 4)) * 5)");
}

TEST(ParserExpressionTest, Return_ParenthesesOverrideLeftGrouping) {
	// Without the parentheses this is ((10 - 4) - 3) - 2.
	EXPECT_EQ(returnedShapeOf("return 10 - (4 - 3) - 2;"), "((10 - (4 - 3)) - 2)");
	EXPECT_EQ(returnedShapeOf("return 1 + (2 + (3 + 4));"), "(1 + (2 + (3 + 4)))");
}

TEST(ParserExpressionTest, Return_ChainsInsideAndOutsideParentheses) {
	EXPECT_EQ(returnedShapeOf("return (8 / 2 / 2) + (1 + 2 + 3) * 2 - 1;"),
	          "((((8 / 2) / 2) + (((1 + 2) + 3) * 2)) - 1)");
}

TEST(ParserExpressionTest, LetThenReturn_ChainsInParentheses) {
	TextParser parser("let x = (1 + 2 + 3) * 2; return (x - 1 - 1) / 2;");

	auto parser_result = parser.parse();
	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	ASSERT_EQ(program->readStatements().size(), 2u);

	auto let = static_cast<const ASTLetNode*>(program->readStatements()[0]);
	EXPECT_EQ(shapeOf(let->readExpression()), "(((1 + 2) + 3) * 2)");
	auto ret = static_cast<const ASTReturnNode*>(program->readStatements()[1]);
	EXPECT_EQ(shapeOf(ret->readExpression()), "(((x - 1) - 1) / 2)");
}

// ---- Comparisons inside parentheses ----
//
// Parentheses hold any expression, a comparison included. A parenthesised
// comparison is then an ordinary operand: it can sit on either side of
// another operator.

TEST(ParserExpressionTest, Parentheses_HoldAComparison) {
	EXPECT_EQ(returnedShapeOf("return (1 < 2);"), "(1 < 2)");
	EXPECT_EQ(returnedShapeOf("return (a == b);"), "(a == b)");
}

TEST(ParserExpressionTest, Parentheses_NestedAroundAComparison) {
	EXPECT_EQ(returnedShapeOf("return ((1 != 2));"), "(1 != 2)");
}

TEST(ParserExpressionTest, Parentheses_ComparisonWithChainsOnBothSides) {
	EXPECT_EQ(returnedShapeOf("return (1 + 2 + 3 > 2 * 3);"), "(((1 + 2) + 3) > (2 * 3))");
}

TEST(ParserExpressionTest, Parentheses_TwoComparisonsCompared) {
	// Not a chain: each side is one parenthesised operand.
	EXPECT_EQ(returnedShapeOf("return (1 < 2) == (3 < 4);"), "((1 < 2) == (3 < 4))");
}

TEST(ParserExpressionTest, Parentheses_ComparisonAsAnArithmeticOperand) {
	EXPECT_EQ(returnedShapeOf("return (1 < 2) + 1;"), "((1 < 2) + 1)");
	EXPECT_EQ(returnedShapeOf("return 2 * (a > b);"), "(2 * (a > b))");
}

TEST(ParserExpressionTest, Parentheses_ComparisonInLetAndWhile) {
	EXPECT_TRUE(TextParser("let x = (1 < 2); return x;").parse().has_value());
	EXPECT_TRUE(TextParser("let i = 0; while ((i < 5)) { i++; } return i;").parse().has_value());
}

TEST(ParserExpressionTest, Parentheses_ChainedComparisonInsideIsStillAnError) {
	auto result = TextParser("return (1 < 2 < 3);").parse();

	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error().code, ErrorCode::SYNTAX_ERROR_CHAINED_COMPARISON);
}

TEST(ParserExpressionTest, Parentheses_UnclosedAroundAComparison) {
	auto result = TextParser("return (1 < 2;").parse();

	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error().code, ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH);
}

// ---- Unary minus ----
//
// `-x` negates x. It binds tighter than `*` and `/`, and it can repeat.
//
// The tree stores a negation as a unary node holding its operand, written
// here as "(-x)". The values it produces are checked by running whole
// programs, in test/programs/tests_programs.cpp.

TEST(ParserExpressionTest, UnaryMinus_Shape) {
	EXPECT_EQ(parsedShapeOf("-5"), "(-5)");
	EXPECT_EQ(parsedShapeOf("-(2 + 3)"), "(-(2 + 3))");
	EXPECT_EQ(parsedShapeOf("- -4"), "(-(-4))");
}

TEST(ParserExpressionTest, UnaryMinus_BindsTighterThanBinaryOperators) {
	EXPECT_EQ(parsedShapeOf("-2 * 3"), "((-2) * 3)");
	EXPECT_EQ(parsedShapeOf("-2 - 3"), "((-2) - 3)");
	EXPECT_EQ(parsedShapeOf("3 - -2"), "(3 - (-2))");
	EXPECT_EQ(parsedShapeOf("2 * -a"), "(2 * (-a))");
}

TEST(ParserExpressionTest, UnaryMinus_SourceRangeStartsAtTheOperator) {
	// tokens: return(0) -(1) ((2) 2(3) +(4) 3(5) )(6) ;(7)
	TextParser parser("return -(2 + 3);");

	auto parser_result = parser.parse();
	ASSERT_TRUE(parser_result.has_value());
	auto expression = returnedExpression(static_cast<const ASTProgramNode*>(*parser_result));
	ASSERT_NE(expression, nullptr);
	ASSERT_EQ(expression->readType(), ASTNodeType::UNARY_EXPRESSION);

	EXPECT_EQ(expression->readSourceRange().first, 1u);
	EXPECT_EQ(expression->readSourceRange().last, 6u);
}

TEST(ParserExpressionTest, UnaryMinus_Parses) {
	EXPECT_TRUE(TextParser("return -5;").parse().has_value());
	EXPECT_TRUE(TextParser("let x = 5; return -x;").parse().has_value());
	EXPECT_TRUE(TextParser("return -(2 + 3);").parse().has_value());
}

TEST(ParserExpressionTest, UnaryMinus_AfterABinaryOperator) {
	EXPECT_TRUE(TextParser("return 3 - -2;").parse().has_value());
	EXPECT_TRUE(TextParser("return 3 * -2;").parse().has_value());
	EXPECT_TRUE(TextParser("return 1 < -2;").parse().has_value());
}

TEST(ParserExpressionTest, UnaryMinus_Repeats) {
	// With a space: `--` without one is the decrement operator.
	EXPECT_TRUE(TextParser("return - -4;").parse().has_value());
	EXPECT_TRUE(TextParser("return -(-4);").parse().has_value());
}

TEST(ParserExpressionTest, UnaryMinus_NeedsAnOperand) {
	auto result = TextParser("return -;").parse();

	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error().code, ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION);
}

TEST(ParserExpressionTest, UnaryMinus_AloneIsNotAStatement) {
	// A negation does nothing on its own, like any other bare expression.
	auto result = TextParser("let x = 1; -x;").parse();

	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error().code, ErrorCode::SYNTAX_ERROR_UNUSED_EXPRESSION);
}
