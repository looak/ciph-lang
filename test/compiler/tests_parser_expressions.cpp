// Parser tests for arithmetic expressions: which operator binds tighter, and
// how a chain of operators at the same level is grouped.

#include <string>

#include <fmt/core.h>
#include <gtest/gtest.h>

#include <ciph/compiler/ast.hpp>
#include <ciph/compiler/parser.hpp>

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
	Parser parser(code);
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
	Parser parser(code);

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
	Parser parser(code);

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
	Parser parser(code);

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
	Parser parser("let x = 1 + 2 + 3; return x - 1 - 1;");

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
	Parser parser("return 1 + 2 + 3 == 2 * 3 * 1;");

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
	Parser parser("return 1 + 2 + 3;");

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
	Parser parser("let x = (1 + 2 + 3) * 2; return (x - 1 - 1) / 2;");

	auto parser_result = parser.parse();
	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	ASSERT_EQ(program->readStatements().size(), 2u);

	auto let = static_cast<const ASTLetNode*>(program->readStatements()[0]);
	EXPECT_EQ(shapeOf(let->readExpression()), "(((1 + 2) + 3) * 2)");
	auto ret = static_cast<const ASTReturnNode*>(program->readStatements()[1]);
	EXPECT_EQ(shapeOf(ret->readExpression()), "(((x - 1) - 1) / 2)");
}
