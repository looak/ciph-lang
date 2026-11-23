#include <gtest/gtest.h>

#include <utility>
#include <vector>

#include <ciph/compiler/ast.hpp>
#include <ciph/compiler/compile_unit.hpp>
#include <ciph/compiler/error_defines.hpp>

using namespace ciph;

TEST(CompileUnitTest, ValidSource_ParsesWithoutDiagnostics) {
    CompileUnit unit("let x = 5; return x;");

    EXPECT_TRUE(unit.parse());

    EXPECT_FALSE(unit.hasErrors());
    ASSERT_NE(unit.astRoot(), nullptr);
    EXPECT_EQ(unit.astRoot()->readType(), ASTNodeType::PROGRAM);
    EXPECT_FALSE(unit.readTokens().empty());
}

TEST(CompileUnitTest, SyntaxError_IsReportedAsDiagnostic) {
    CompileUnit unit("let x = 5");

    EXPECT_FALSE(unit.parse());

    EXPECT_EQ(unit.astRoot(), nullptr);
    ASSERT_EQ(unit.readDiagnostics().size(), 1u);
    EXPECT_EQ(unit.readDiagnostics()[0].code, ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON);
}

TEST(CompileUnitTest, SeveralUnits_KeepTheirOwnDiagnostics) {
    std::vector<CompileUnit> units;
    units.emplace_back("let x = 5; return x;");
    units.emplace_back("let = 5;");

    EXPECT_TRUE(units[0].parse());
    EXPECT_FALSE(units[1].parse());

    EXPECT_FALSE(units[0].hasErrors());
    ASSERT_EQ(units[1].readDiagnostics().size(), 1u);
    EXPECT_EQ(units[1].readDiagnostics()[0].code, ErrorCode::SYNTAX_ERROR_EXPECTED_IDENTIFIER);
}

TEST(CompileUnitTest, MovingAUnit_KeepsTheSourceWhereItWas) {
    CompileUnit unit("let x = 5; return x;");
    const char* before = unit.readSource().data();

    CompileUnit moved = std::move(unit);

    EXPECT_EQ(moved.readSource().data(), before);
}

TEST(CompileUnitTest, ValidSource_GeneratesBytecode) {
    CompileUnit unit("let x = 5; return x;");

    ASSERT_TRUE(unit.parse());
    ASSERT_TRUE(unit.semanticAnalyze());
    EXPECT_TRUE(unit.generateCode());

    EXPECT_TRUE(unit.hasGenerated());
    EXPECT_FALSE(unit.readBytecode().empty());
}

TEST(CompileUnitTest, SemanticError_StopsCodeGeneration) {
    CompileUnit unit("return y;");

    ASSERT_TRUE(unit.parse());
    EXPECT_FALSE(unit.semanticAnalyze());
    EXPECT_FALSE(unit.generateCode());

    EXPECT_FALSE(unit.hasGenerated());
    EXPECT_TRUE(unit.readBytecode().empty());
    ASSERT_EQ(unit.readDiagnostics().size(), 1u);
    EXPECT_EQ(unit.readDiagnostics()[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
}

TEST(CompileUnitTest, CodeGeneratorError_IsReportedAndLeavesNoBytecode) {
    // Functions pass the analyzer but cannot be compiled yet.
    CompileUnit unit("fn five() { return 5; }");

    ASSERT_TRUE(unit.parse());
    ASSERT_TRUE(unit.semanticAnalyze());
    EXPECT_FALSE(unit.generateCode());

    EXPECT_FALSE(unit.hasGenerated());
    EXPECT_TRUE(unit.readBytecode().empty());
    ASSERT_EQ(unit.readDiagnostics().size(), 1u);
    EXPECT_EQ(unit.readDiagnostics()[0].code, ErrorCode::CODEGEN_ERROR_UNSUPPORTED);
}

TEST(CompileUnitTest, StagesOutOfOrder_DoNothing) {
    CompileUnit unit("let x = 5; return x;");

    EXPECT_FALSE(unit.semanticAnalyze());
    EXPECT_FALSE(unit.generateCode());

    EXPECT_FALSE(unit.hasAnalyzed());
    EXPECT_FALSE(unit.hasGenerated());
}
