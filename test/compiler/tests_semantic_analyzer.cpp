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

    SemanticAnalyzer analyzer(static_cast<const ASTProgramNode*>(*parser_result));
    return analyzer.analyze();
}

} // namespace

TEST(SemanticAnalyzerTest, ValidProgram_NoErrors) {
    auto errors = analyze("let x = 5; return x;");

    EXPECT_TRUE(errors.empty());
}

// The tests below describe the first rules. Remove DISABLED_ from each one as
// the rule it covers is implemented.

TEST(SemanticAnalyzerTest, DISABLED_UndeclaredIdentifier_IsAnError) {
    auto errors = analyze("return x;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, DISABLED_UseBeforeDeclaration_IsAnError) {
    auto errors = analyze("let y = x; let x = 5; return y;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, DISABLED_RedeclaredIdentifier_IsAnError) {
    auto errors = analyze("let x = 5; let x = 6; return x;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, DISABLED_RedeclaredInNestedBlock_IsAnError) {
    auto errors = analyze("let x = 0; while (x < 5) { let x = 1; } return x;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, DISABLED_DeclarationEndsWithItsBlock) {
    auto errors = analyze("let i = 0; while (i < 5) { let inner = 1; i++; } return inner;");

    ASSERT_EQ(errors.size(), 1);
    EXPECT_EQ(errors[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(SemanticAnalyzerTest, DISABLED_SameNameInSiblingBlocks_IsAllowed) {
    auto errors = analyze("let i = 0; while (i < 5) { let t = 1; i++; } while (i < 9) { let t = 2; i++; } return i;");

    EXPECT_TRUE(errors.empty());
}

TEST(SemanticAnalyzerTest, DISABLED_AllErrorsAreCollected) {
    auto errors = analyze("let x = a; let y = b; return x;");

    EXPECT_EQ(errors.size(), 2);
}

TEST(SemanticAnalyzerTest, DISABLED_CallBeforeDefinition_IsAllowed) {
    auto errors = analyze("let x = number(); fn number() { return 42; } return x;");

    EXPECT_TRUE(errors.empty());
}
