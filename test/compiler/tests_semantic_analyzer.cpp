#include <gtest/gtest.h>

#include <ciph/compiler/ast.hpp>
#include <ciph/compiler/error_defines.hpp>
#include <ciph/compiler/parser.hpp>
#include <ciph/compiler/semantic_analyzer.hpp>

using namespace ciph;

namespace {

// Parses `code`, which must be syntactically valid, and returns what the
// analyzer reports for it.
std::vector<ParserError>
analyze(const std::string& code) {
    Parser parser(code);
    auto parser_result = parser.parse();
    EXPECT_TRUE(parser_result.has_value()) << "test source did not parse";
    if (!parser_result) {
        return {};
    }

    SemanticAnalyzer analyzer(static_cast<const ASTProgramNode*>(*parser_result), parser.readTokens());
    return analyzer.analyze();
}

} // namespace

TEST(SemanticAnalyzerTest, ValidProgram_NoErrors) {
    auto errors = analyze("let x = 5; return x;");

    EXPECT_TRUE(errors.empty());
}

TEST(SemanticAnalyzerTest, UndeclaredIdentifier_IsAnError) {
    auto errors = analyze("return x;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, UseBeforeDeclaration_IsAnError) {
    auto errors = analyze("let y = x; let x = 5; return y;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, RedeclaredIdentifier_IsAnError) {
    auto errors = analyze("let x = 5; let x = 6; return x;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, RedeclaredInNestedBlock_IsAnError) {
    auto errors = analyze("let x = 0; while (x < 5) { let x = 1; } return x;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, DeclarationEndsWithItsBlock) {
    auto errors = analyze("let i = 0; while (i < 5) { let inner = 1; i++; } return inner;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, SameNameInSiblingBlocks_IsAllowed) {
    auto errors = analyze("let i = 0; while (i < 5) { let t = 1; i++; } while (i < 9) { let t = 2; i++; } return i;");

    EXPECT_TRUE(errors.empty());
}

TEST(SemanticAnalyzerTest, AllErrorsAreCollected) {
    auto errors = analyze("let x = a; let y = b; return x;");

    EXPECT_EQ(errors.size(), 2);
}

TEST(SemanticAnalyzerTest, CallBeforeDefinition_IsAllowed) {
    auto errors = analyze("let x = number(); fn number() { return 42; } return x;");

    EXPECT_TRUE(errors.empty());
}

TEST(SemanticAnalyzerTest, CallToUnknownFunction_IsAnError) {
    auto errors = analyze("return nope();");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, RedefinedFunction_IsAnError) {
    auto errors = analyze("fn a() { return 1; } fn a() { return 2; } return a();");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, FunctionBodyCannotSeeOuterVariables) {
    auto errors = analyze("let x = 5; fn f() { return x; } return f();");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, FunctionCanCallItself) {
    auto errors = analyze("fn f() { return f(); } return f();");

    EXPECT_TRUE(errors.empty());
}

TEST(SemanticAnalyzerTest, FunctionEndsWithItsBlock) {
    auto errors = analyze("let i = 0; while (i < 1) { fn g() { return 1; } i++; } return g();");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, FunctionsAndVariablesHaveSeparateNames) {
    auto errors = analyze("let f = 1; fn f() { return 2; } return f + f();");

    EXPECT_TRUE(errors.empty());
}

TEST(SemanticAnalyzerTest, LetCannotUseItsOwnName) {
    auto errors = analyze("let x = x; return x;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, ErrorPointsAtTheName) {
    auto errors = analyze("let x = 5;\nreturn y;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].position.line, 2);
    EXPECT_EQ(errors[0].position.column, 8);
}
