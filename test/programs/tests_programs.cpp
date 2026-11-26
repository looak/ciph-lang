// Whole-program tests: source text goes through the compiler and the result
// runs in the VM. They check what a program returns, and say nothing about
// how the tree or the bytecode looks.

#include <optional>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <gtest/gtest.h>

#include <ciph/compiler/compile_unit.hpp>
#include <ciph/vm/processing_unit.hpp>

using namespace ciph;

namespace {

// Compiles and runs `source`. Returns what the program returned, or nothing
// if it did not compile; the first diagnostic is then printed with the test.
std::optional<int16_t>
run(const std::string& source) {
    CompileUnit unit(source);
    if (!unit.parse() || !unit.semanticAnalyze() || !unit.generateCode()) {
        if (unit.hasErrors()) {
            const Diagnostic& diagnostic = unit.readDiagnostics().front();
            ADD_FAILURE() << "did not compile: line " << diagnostic.location.span.start.line << ", column "
                          << diagnostic.location.span.start.column << ": " << diagnostic.message << "\n  in: " << source;
        }
        return std::nullopt;
    }

    std::vector<uint8_t> bytecode = unit.readBytecode();
    ProcessingUnit vm;
    vm.load_program(bytecode.data(), static_cast<uint16_t>(bytecode.size()));
    return vm.execute();
}

} // namespace

// ---- What already works ----

TEST(ProgramTest, ReturnsALiteral) {
    EXPECT_EQ(run("return 42;"), 42);
}

TEST(ProgramTest, ChainedArithmetic) {
    EXPECT_EQ(run("return 10 - 4 - 3;"), 3);
    EXPECT_EQ(run("return (1 + 2 + 3) * 2;"), 12);
}

TEST(ProgramTest, WhileLoopCountsToFive) {
    EXPECT_EQ(run("let i = 0; while (i < 5) { i++; } return i;"), 5);
}

// ---- Variable slots ----
//
// Every `let` gets its own slot on the stack. Reading a variable while
// working out a value must not throw off which slot the next `let` gets.

TEST(ProgramTest, VariableSlots_LiteralsOnly) {
    EXPECT_EQ(run("let x = 1; let y = 2; let z = 5; return z;"), 5);
}

TEST(ProgramTest, VariableSlots_AfterALetThatCopiesAVariable) {
    EXPECT_EQ(run("let x = 1; let y = x; let z = 5; return z;"), 5);
    EXPECT_EQ(run("let x = 1; let y = x; let z = 5; return y + z;"), 6);
}

TEST(ProgramTest, VariableSlots_AfterALetThatComputesWithAVariable) {
    EXPECT_EQ(run("let x = 1; let y = x + 1; let z = 5; return z;"), 5);
}

TEST(ProgramTest, VariableSlots_AfterAWhileConditionThatReadsAVariable) {
    EXPECT_EQ(run("let i = 0; while (i < 3) { i++; } let z = 7; return z;"), 7);
}

TEST(ProgramTest, VariableSlots_AfterALetThatNegatesAVariable) {
    EXPECT_EQ(run("let x = 1; let y = -x; let z = 5; return z;"), 5);
}

// ---- Unary minus ----

TEST(ProgramTest, UnaryMinus_OnALiteral) {
    EXPECT_EQ(run("return -5;"), -5);
}

TEST(ProgramTest, UnaryMinus_OnAVariable) {
    EXPECT_EQ(run("let x = 5; return -x;"), -5);
}

TEST(ProgramTest, UnaryMinus_OnParentheses) {
    EXPECT_EQ(run("return -(2 + 3);"), -5);
}

TEST(ProgramTest, UnaryMinus_AfterSubtraction) {
    EXPECT_EQ(run("return 3 - -2;"), 5);
}

TEST(ProgramTest, UnaryMinus_Twice) {
    EXPECT_EQ(run("return - -4;"), 4);
}

TEST(ProgramTest, UnaryMinus_BindsTighterThanMultiplication) {
    // (-2) * 3 and -(2 * 3) are both -6, so use a case that tells them
    // apart: (-2) - 3 is -5, while -(2 - 3) is 1.
    EXPECT_EQ(run("return -2 - 3;"), -5);
    EXPECT_EQ(run("return -2 * 3 + 10;"), 4);
}

TEST(ProgramTest, UnaryMinus_InsideAChain) {
    EXPECT_EQ(run("return 10 - -4 - 3;"), 11);
}

TEST(ProgramTest, UnaryMinus_InALet) {
    EXPECT_EQ(run("let x = -7; let y = -x; return y + x;"), 0);
}
