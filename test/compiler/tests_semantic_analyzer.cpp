#include <gtest/gtest.h>

#include <ciph/compiler/ast.hpp>
#include <ciph/compiler/diagnostics.hpp>
#include <ciph/compiler/parser.hpp>
#include <ciph/compiler/semantic_analyzer.hpp>

using namespace ciph;

namespace {

// Parses `code`, which must be syntactically valid, and returns what the
// analyzer reports for it.
std::vector<Diagnostic>
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
    EXPECT_EQ(errors[0].location.span.start.line, 2);
    EXPECT_EQ(errors[0].location.span.start.column, 8);
}

TEST(SemanticAnalyzerTest, ErrorsAreReportedInSourceOrder) {
    // Function definitions are checked before the statements around them, so
    // the redefinition is found before the undeclared x, but comes later in
    // the source.
    auto errors = analyze("return x; fn a() { return 1; } fn a() { return 2; }");

    ASSERT_EQ(errors.size(), 2);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
    EXPECT_EQ(errors[1].code, ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, NestedFunctionWithOuterName_IsAnError) {
    auto errors = analyze("fn f() { fn f() { return 1; } return 2; } return f();");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, FunctionBodyMayReuseOuterVariableName) {
    // The body cannot see the outer x, so declaring its own x is not a
    // redeclaration.
    auto errors = analyze("let x = 5; fn f() { let x = 1; return x; } return f();");

    EXPECT_TRUE(errors.empty());
}

TEST(SemanticAnalyzerTest, UndeclaredNameInWhileCondition_IsAnError) {
    auto errors = analyze("while (y < 5) { } return 0;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, RedefinedFunction_PointsAtTheName) {
    // The function's range is closed at its '}', after the focus is set, so
    // this also checks that closing a range keeps the focus.
    auto errors = analyze("fn a() { return 1; }\nfn a() { return 2; } return a();");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].location.span.start.line, 2);
    EXPECT_EQ(errors[0].location.span.start.column, 4);
    EXPECT_NE(errors[0].message.find("line 1, column 4"), std::string::npos) << errors[0].message;
}

TEST(SemanticAnalyzerTest, RedeclaredVariable_PointsAtTheName) {
    auto errors = analyze("let x = 5;\nlet x = 6; return x;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].location.span.start.line, 2);
    EXPECT_EQ(errors[0].location.span.start.column, 5);
    EXPECT_NE(errors[0].message.find("line 1, column 5"), std::string::npos) << errors[0].message;
}
