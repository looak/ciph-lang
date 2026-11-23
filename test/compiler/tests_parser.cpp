#include <fmt/core.h>
#include <gtest/gtest.h>

#include <ciph/compiler/ast.hpp>
#include <ciph/compiler/error_defines.hpp>
#include <ciph/compiler/parser.hpp>

using namespace ciph;

TEST(ParserTest, ReturnStatement_DefaultNumericNode_ValueZero) {
	// setup
	std::string code("return;");
	Parser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto program_node = parser_result.value();
	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);
	auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);

	EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::NUMERIC_LITERAL);
	auto numericNode = static_cast<const ASTNumericLiteralNode*>(returnNode->readExpression());
	EXPECT_EQ(numericNode->readValue(), 0);
}

TEST(ParserTest, ReturnStatement_NumericNodeGivenValue) {
	// setup
	std::string code("return 5;");
	Parser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto program_node = parser_result.value();
	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);
	auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);

	EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::NUMERIC_LITERAL);
	auto numericNode = static_cast<const ASTNumericLiteralNode*>(returnNode->readExpression());
	EXPECT_EQ(numericNode->readValue(), 5);
}

TEST(ParserTest, ReturnStatement_NumericNodeExpression) {
	// setup
	std::string code("return 5 + 2;");
	Parser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto program_node = parser_result.value();
	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);
	auto returnNode = static_cast<const ASTReturnNode*>(result->readStatements()[0]);

	EXPECT_EQ(returnNode->readExpression()->readType(), ASTNodeType::BINARY_EXPRESSION);
}

TEST(ParserTest, LetStatement) {
	// setup
	std::string code("let x = 5;");
	Parser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto program_node = parser_result.value();
	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::LET);
	auto letNode = static_cast<const ASTLetNode*>(result->readStatements()[0]);
	EXPECT_EQ(letNode->readExpression()->readType(), ASTNodeType::NUMERIC_LITERAL);
	EXPECT_EQ(letNode->readIdentifier(), "x");
}

TEST(ParserTest, LetStatement_ErrorInExpression) {
	// setup
	std::string code("let x 42;"); // should produce an error
	Parser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto error = parser_result.error();

	EXPECT_EQ(error.code, ErrorCode::SYNTAX_ERROR_EXPECTED_OPERATOR);
}

TEST(ParserTest, LetStatement_Expression) {
	// setup
	std::string code("let x = 5 + 5;");
	Parser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto program_node = parser_result.value();
	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 1);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::LET);
	auto letNode = static_cast<const ASTLetNode*>(result->readStatements()[0]);
	EXPECT_EQ(letNode->readExpression()->readType(), ASTNodeType::BINARY_EXPRESSION);
	EXPECT_EQ(letNode->readIdentifier(), "x");

	auto binaryExpression = static_cast<const ASTBinaryExpressionNode*>(letNode->readExpression());
	EXPECT_EQ(binaryExpression->readOperator(), OperatorType::ADDITION);
}

TEST(ParserTest, IncStatement) {
	// setup
	std::string code(R"(let x = 5;
						x++;)");
	Parser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto program_node = parser_result.value();
	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 2);
	EXPECT_EQ(result->readStatements()[1]->readType(), ASTNodeType::IDENTIFIER);
	auto identifier = static_cast<const ASTIdentifierNode*>(result->readStatements()[1]);
	EXPECT_EQ(identifier->readIncDec(), IncDec::PostIncrement);
	EXPECT_TRUE(identifier->readHasSideEffect());
}

TEST(ParserTest, EqualsOperator) {
	// setup
	std::string code("return x == x;");
	Parser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto program_node = parser_result.value();
	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	ASSERT_EQ(result->readStatements().size(), 1);
	ASSERT_EQ(result->readStatements()[0]->readType(), ASTNodeType::RETURN);
	auto expression = static_cast<const ASTReturnNode*>(result->readStatements()[0])->readExpression();
	ASSERT_EQ(expression->readType(), ASTNodeType::COMPARISON_EXPRESSION);
	auto compare_expression = static_cast<const ASTComparisonExpressionNode*>(expression);
	EXPECT_EQ(compare_expression->readOperator(), OperatorType::EQUAL);
}

// DISABLED: placeholder. Chained comparisons such as `x == x == x` should be
// rejected with an error, but the parser does not do that yet and this test
// has no real assertion. Re-enable once the error exists.
TEST(ParserTest, DISABLED_EqualsOperator_NotLegal) {
	// setup
	std::string code("return x == x == x;");
	Parser parser(code);

	// do - we know the node type returned is a program
	// auto parser_result = parser.parse();
	// auto program_node = parser_result.value();
	// ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_TRUE(false) << "This test should fail and produce a error message";
}

TEST(ParserTest, WhileStatement_Legal) {
	// setup
	std::string code(
		R"(
			let i = 0;
			while (i < 5) {
				i++;
			}
			return i;)");
	Parser parser(code);

	// do - we know the node type returned is a program
	auto parser_result = parser.parse();
	auto program_node = parser_result.value();
	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 3);
	EXPECT_EQ(result->readStatements()[1]->readType(), ASTNodeType::WHILE);
}

TEST(ParserTest, SimpleFunction_ReturnsConstant) {
		// setup
	std::string code(
		R"(
			fn number() {
				return 42;
			}
			return number();)");
	Parser parser(code);

	auto parser_result = parser.parse();
	ASSERT_TRUE(parser_result.has_value()) << "Parser produced an error: " << parser_result.error().additionalInfo;
	ASTProgramNode* program_node = static_cast<ASTProgramNode*>(parser_result.value());

	ASTProgramNode* result = static_cast<ASTProgramNode*>(program_node);

	// validate
	EXPECT_EQ(result->readType(), ASTNodeType::PROGRAM);

	EXPECT_EQ(result->readStatements().size(), 2);
	EXPECT_EQ(result->readStatements()[0]->readType(), ASTNodeType::FUNCTION);
	EXPECT_EQ(result->readStatements()[1]->readType(), ASTNodeType::RETURN);
	EXPECT_EQ(static_cast<const ASTReturnNode*>(result->readStatements()[1])->readExpression()->readType(), ASTNodeType::CALL_EXPRESSION);
}

// Simple statements must end with ';', including the last one in a block.
// Block statements (while, fn) take none; the tests above cover that.

TEST(ParserTest, MissingSemicolon_AfterLet) {
	Parser parser("let x = 5");

	auto parser_result = parser.parse();

	ASSERT_FALSE(parser_result.has_value());
	EXPECT_EQ(parser_result.error().code, ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON);
}

TEST(ParserTest, MissingSemicolon_AfterReturn) {
	Parser parser("return 5");

	auto parser_result = parser.parse();

	ASSERT_FALSE(parser_result.has_value());
	EXPECT_EQ(parser_result.error().code, ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON);
}

TEST(ParserTest, MissingSemicolon_AfterExpression) {
	Parser parser("number()");

	auto parser_result = parser.parse();

	ASSERT_FALSE(parser_result.has_value());
	EXPECT_EQ(parser_result.error().code, ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON);
}

// An expression can stand alone as a statement only when it does something:
// a call, or `++` / `--`. Anything else computes a value that nobody uses,
// and is a syntax error.
//
// Rejected statements report SYNTAX_ERROR_UNUSED_EXPRESSION.

namespace {

void
expectUnusedExpression(const char* code) {
	auto parser_result = Parser(code).parse();
	ASSERT_FALSE(parser_result.has_value()) << "Parsed without error: " << code;
	EXPECT_EQ(parser_result.error().code, ErrorCode::SYNTAX_ERROR_UNUSED_EXPRESSION) << code;
}

} // namespace

TEST(ParserTest, ExpressionStatement_CallIsAllowed) {
	Parser parser("fn number() { return 42; } number(); return 0;");

	auto parser_result = parser.parse();

	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	ASSERT_EQ(program->readStatements().size(), 3u);
	EXPECT_EQ(program->readStatements()[1]->readType(), ASTNodeType::CALL_EXPRESSION);
}

TEST(ParserTest, ExpressionStatement_IncrementAndDecrementAreAllowed) {
	Parser parser("let i = 0; i++; i--; return i;");

	auto parser_result = parser.parse();

	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	EXPECT_EQ(program->readStatements().size(), 4u);
}

TEST(ParserTest, ExpressionStatement_ArithmeticIsRejected) {
	expectUnusedExpression("1 + 2;");
	expectUnusedExpression("2 * (2 + 3);");
}

TEST(ParserTest, ExpressionStatement_ComparisonIsRejected) {
	expectUnusedExpression("let x = 1; x == x;");
}

TEST(ParserTest, ExpressionStatement_LoneValueIsRejected) {
	expectUnusedExpression("5;");
	expectUnusedExpression("let x = 1; x;");
	expectUnusedExpression("(5);");
}

TEST(ParserTest, ExpressionStatement_CallOrIncrementInsideALargerExpressionIsRejected) {
	// The statement as a whole has to be the call or the increment.
	expectUnusedExpression("fn number() { return 42; } number() + 1;");
	expectUnusedExpression("let i = 0; i++ + 1;");
}

TEST(ParserTest, ExpressionStatement_RejectedInsideABlock) {
	expectUnusedExpression("let i = 0; while (i < 5) { i + 1; i++; } return i;");
	expectUnusedExpression("fn number() { 1 + 2; return 42; } return number();");
}

TEST(ParserTest, IncDec_PostDecrementIsRecorded) {
	auto parser_result = Parser("let i = 0; i--; return i;").parse();

	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	auto decrement = static_cast<const ASTIdentifierNode*>(program->readStatements()[1]);
	EXPECT_EQ(decrement->readIncDec(), IncDec::PostDecrement);
	EXPECT_TRUE(decrement->readHasSideEffect());
}

TEST(ParserTest, IncDec_PlainIdentifierHasNone) {
	auto parser_result = Parser("let i = 0; return i;").parse();

	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	auto returnNode = static_cast<const ASTReturnNode*>(program->readStatements()[1]);
	auto identifier = static_cast<const ASTIdentifierNode*>(returnNode->readExpression());
	EXPECT_EQ(identifier->readIncDec(), IncDec::None);
	EXPECT_FALSE(identifier->readHasSideEffect());
}

TEST(ParserTest, IncDec_PrefixFormIsNotSupported) {
	// Only `x++` / `x--` exist; `++` cannot start an expression.
	for (const char* code : {"let i = 0; ++i; return i;", "let i = 0; return --i;"}) {
		auto parser_result = Parser(code).parse();
		ASSERT_FALSE(parser_result.has_value()) << code;
		EXPECT_EQ(parser_result.error().code, ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION) << code;
	}
}

TEST(ParserTest, Origin_ReturnWithoutValueIsSynthesized) {
	auto parser_result = Parser("return;").parse();

	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	auto returnNode = static_cast<const ASTReturnNode*>(program->readStatements()[0]);
	EXPECT_EQ(returnNode->readOrigin(), Origin::Text);
	EXPECT_EQ(returnNode->readExpression()->readOrigin(), Origin::Synthesized);
}

TEST(ParserTest, Origin_ParsedValueIsText) {
	auto parser_result = Parser("return 5;").parse();

	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	auto returnNode = static_cast<const ASTReturnNode*>(program->readStatements()[0]);
	EXPECT_EQ(returnNode->readExpression()->readOrigin(), Origin::Text);
}

TEST(ParserTest, MissingSemicolon_BetweenStatements) {
	Parser parser("let x = 5 return x;");

	auto parser_result = parser.parse();

	ASSERT_FALSE(parser_result.has_value());
	EXPECT_EQ(parser_result.error().code, ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON);
}

TEST(ParserTest, MissingSemicolon_LastStatementInBlock) {
	Parser parser("let i = 0; while (i < 5) { i++ } return i;");

	auto parser_result = parser.parse();

	ASSERT_FALSE(parser_result.has_value());
	EXPECT_EQ(parser_result.error().code, ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON);
}

TEST(ParserTest, MissingSemicolon_InsideFunctionBody) {
	Parser parser("fn number() { return 42 } return number();");

	auto parser_result = parser.parse();

	ASSERT_FALSE(parser_result.has_value());
	EXPECT_EQ(parser_result.error().code, ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON);
}

TEST(ParserTest, LoneSemicolon_IsAnError) {
	Parser parser("let x = 5;;");

	auto parser_result = parser.parse();

	ASSERT_FALSE(parser_result.has_value());
	EXPECT_EQ(parser_result.error().code, ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION);
}

TEST(ParserTest, MissingSemicolon_PointsJustPastTheStatement) {
	// The ';' belongs after the 5, at line 1 column 10, not at `return` on
	// the next line where the parser noticed it was missing.
	Parser parser("let x = 5\nreturn x;");

	auto parser_result = parser.parse();

	ASSERT_FALSE(parser_result.has_value());
	EXPECT_EQ(parser_result.error().position.line, 1u);
	EXPECT_EQ(parser_result.error().position.column, 10u);
}

// ---- Missing operands: a token that cannot start an expression is an error ----
//
// These used to parse "successfully" with a null node where the operand
// belonged, which later stages would then dereference.

namespace {

void
expectMissingExpression(const char* code) {
	auto parser_result = Parser(code).parse();
	ASSERT_FALSE(parser_result.has_value()) << "Parsed without error: " << code;
	EXPECT_EQ(parser_result.error().code, ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION) << code;
}

} // namespace

TEST(ParserTest, MissingOperand_AfterAdditiveOperator) {
	expectMissingExpression("return 1 + ;");
	expectMissingExpression("return 1 - ;");
	expectMissingExpression("return 1 + 2 + ;");
}

TEST(ParserTest, MissingOperand_AfterMultiplicativeOperator) {
	expectMissingExpression("return 2 * ;");
	expectMissingExpression("return 2 / ;");
	expectMissingExpression("return 1 + 2 * ;");
}

TEST(ParserTest, MissingOperand_AfterComparisonOperator) {
	expectMissingExpression("return 1 < ;");
	expectMissingExpression("return 1 == ;");
}

TEST(ParserTest, MissingOperand_BeforeBinaryOperator) {
	expectMissingExpression("return * 2;");
	expectMissingExpression("return + 2;");
}

TEST(ParserTest, MissingOperand_LetWithoutValue) {
	expectMissingExpression("let x = ;");
	expectMissingExpression("let x = 1 + ;");
}

TEST(ParserTest, MissingOperand_InsideParentheses) {
	expectMissingExpression("return ();");
	expectMissingExpression("return (1 + );");
}

TEST(ParserTest, MissingOperand_AtEndOfFile) {
	expectMissingExpression("let x = 1 +");
	expectMissingExpression("return");
}

TEST(ParserTest, MissingOperand_StatementThatCannotStartAnExpression) {
	expectMissingExpression("}");
	expectMissingExpression("* 2;");
}

TEST(ParserTest, MissingOperand_PointsAtTheTokenFoundInstead) {
	// The ';' at column 12 is where the right-hand side should have started.
	auto parser_result = Parser("return 1 + ;").parse();

	ASSERT_FALSE(parser_result.has_value());
	EXPECT_EQ(parser_result.error().position.line, 1u);
	EXPECT_EQ(parser_result.error().position.column, 12u);
}

TEST(ParserTest, MissingOperand_EmptyWhileConditionKeepsTheCauseAsANestedError) {
	auto parser_result = Parser("let i = 0; while () { i++; } return i;").parse();

	ASSERT_FALSE(parser_result.has_value());
	EXPECT_EQ(parser_result.error().code, ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION);
	ASSERT_EQ(parser_result.error().nestedError.size(), 1u);
	EXPECT_EQ(parser_result.error().nestedError[0].code, ErrorCode::SYNTAX_ERROR_EXPECTED_EXPRESSION);
}

// ---- Source ranges: which tokens each node was built from ----

namespace {

void
expectRange(const ASTBaseNode* node, uint32_t first, uint32_t last) {
	ASSERT_NE(node, nullptr);
	EXPECT_EQ(node->readSourceRange().first, first);
	EXPECT_EQ(node->readSourceRange().last, last);
}

} // namespace

TEST(ParserTest, SourceRange_CoversTheNodesTokens) {
	// token:       0   1 2 3 4 5 6 7      8 9
	Parser parser("let x = 1 + 2 ; return x ;");

	auto parser_result = parser.parse();
	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);

	// the program is everything except the end-of-file token
	expectRange(program, 0, 9);

	// a statement's range leaves out its ';'
	auto let = static_cast<const ASTLetNode*>(program->readStatements()[0]);
	expectRange(let, 0, 5);

	auto sum = static_cast<const ASTBinaryExpressionNode*>(let->readExpression());
	expectRange(sum, 3, 5);
	expectRange(sum->readLeft(), 3, 3);
	expectRange(sum->readRight(), 5, 5);

	auto ret = static_cast<const ASTReturnNode*>(program->readStatements()[1]);
	expectRange(ret, 7, 8);
	expectRange(ret->readExpression(), 8, 8);
}

TEST(ParserTest, SourceRange_BlockRunsToItsClosingBrace) {
	// token:       0   1 2 3 4 5     6 7 8 9 10 11 12 13 14 15 16     17 18
	Parser parser("let i = 0 ; while ( i < 5 )  {  i  ++ ;  }  return i  ;");

	auto parser_result = parser.parse();
	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);

	auto loop = static_cast<const ASTWhileNode*>(program->readStatements()[1]);
	expectRange(loop, 5, 15);
	expectRange(loop->readCondition(), 7, 9);

	// `i++` is one identifier node whose range covers the name and the `++`
	auto increment = static_cast<const ASTIdentifierNode*>(loop->readStatements()[0]);
	expectRange(increment, 12, 13);
	EXPECT_EQ(increment->readIncDec(), IncDec::PostIncrement);
}

TEST(ParserTest, SourceRange_LeadsBackToLineAndColumn) {
	Parser parser("let x = 5;\n  return x;");

	auto parser_result = parser.parse();
	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	const ASTBaseNode* ret = program->readStatements()[1];

	const Token& first = parser.readTokens()[ret->readSourceRange().first];
	EXPECT_EQ(first.readPosition().line, 2u);
	EXPECT_EQ(first.readPosition().column, 3u);
	EXPECT_EQ(first.readLength(), 6u); // "return"
}

TEST(ParserTest, SourceRange_NodeWithoutSourceHasNoTokens) {
	// `return;` supplies a literal 0 that appears nowhere in the source
	Parser parser("return;");

	auto parser_result = parser.parse();
	ASSERT_TRUE(parser_result.has_value());
	auto program = static_cast<const ASTProgramNode*>(*parser_result);
	auto ret = static_cast<const ASTReturnNode*>(program->readStatements()[0]);

	EXPECT_TRUE(ret->readSourceRange().hasTokens());
	EXPECT_FALSE(ret->readExpression()->readSourceRange().hasTokens());
}
