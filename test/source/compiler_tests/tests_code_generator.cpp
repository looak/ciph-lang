#include <fmt/core.h>
#include <gtest/gtest.h>

#include "ast.hpp"
#include "code_generator.hpp"
#include "lexar_defines.hpp"
#include "parser.hpp"
#include "shared_defines.hpp"

class CodeGeneratorTestFixture : public ::testing::Test {
public:
    void SetUp() override {}

    ASTProgramNode m_program;
};

bool compareBytecode(const uint8_t* expected, const uint8_t* actual, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (expected[i] != actual[i]) {
            return false;
        }
    }
    return true;
}

TEST_F(CodeGeneratorTestFixture, NumericLiteral_ExpectArray) {
    // setup
    ASTNumericLiteralNode* node = new ASTNumericLiteralNode(0x0eef);

    m_program.addStatement(node);

    CodeGenerator generator(&m_program);

    // do
    generator.generateCode();

    // validate
    uint8_t expectedProgram[] = {+instruction::def::PSH_LIT, 0x0E, 0xEF};
    uint32_t expectedSize = sizeof(expectedProgram);

    auto [actualProgram, actualSize] = generator.readRawBytecode();

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, BinaryExpression_ExpectArray) {
    // setup
    ASTNumericLiteralNode* left = new ASTNumericLiteralNode(0x0AFE);
    ASTNumericLiteralNode* right = new ASTNumericLiteralNode(0x0ABE);

    ASTBinaryExpressionNode* node = new ASTBinaryExpressionNode(left, right, OperatorType::ADDITION);

    m_program.addStatement(node);

    CodeGenerator generator(&m_program);

    // do
    generator.generateCode();

    // validate
    uint8_t expectedProgram[] = {+instruction::def::PSH_LIT, 0x0A, 0xFE, +instruction::def::PSH_LIT, 0x0A, 0xBE,
                                 +instruction::def::ADD};

    uint32_t expectedSize = sizeof(expectedProgram);

    auto [actualProgram, actualSize] = generator.readRawBytecode();

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, GeneratingOrderOfOperation_MulBeforeAdd) {
    // setup
    std::string code("2 * 2 + 3");
    // std::string code("(1 + 2 * 3 + 2) / 3");
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = {
        +instruction::def::PSH_LIT, 0, 2, +instruction::def::PSH_LIT, 0, 2, +instruction::def::MUL,
        +instruction::def::PSH_LIT, 0, 3, +instruction::def::ADD};
    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, GeneratingOrderOfOperation_PaarenthasesBeforeMul) {
    // setup
    std::string code("2 * (2 + 3)");
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = {+instruction::def::PSH_LIT,
                                 0,
                                 2,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 2,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 3,
                                 +instruction::def::ADD,
                                 +instruction::def::MUL};
    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, ReturnStatment_Expression) {
    // setup
    std::string code("return 2 * (2 + 3)");
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = {+instruction::def::PSH_LIT,
                                 0,
                                 2,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 2,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 3,
                                 +instruction::def::ADD,
                                 +instruction::def::MUL,
                                 +instruction::def::POP_REG,
                                 +registers::def::ret,
                                 +instruction::def::RET};
    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, ReturnStatment_Expression_Simple) {
    // setup
    std::string code("return 2 + 3");
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = {+instruction::def::PSH_LIT,
                                 0,
                                 2,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 3,
                                 +instruction::def::ADD,
                                 +instruction::def::POP_REG,
                                 +registers::def::ret,
                                 +instruction::def::RET};
    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, LetStatement_Expression) {
    // setup
    std::string code(R"(let x = 2 * (2 + 3)
                        return x)");
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = {+instruction::def::PSH_LIT,
                                 0,
                                 2,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 2,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 3,
                                 +instruction::def::ADD,
                                 +instruction::def::MUL,
                                 +instruction::def::PEK_OFF,
                                 +registers::def::ret,
                                 0,
                                 +instruction::def::RET};

    std::string dissassembly = generator.disassemble();

    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, LetStatement_MultipleVariables) {
    // setup
    std::string code(R"(let x = 2
                        let y = 3
                        let z = x + y
                        return z + y)");
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = {+instruction::def::PSH_LIT,
                                 0,
                                 2,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 3,
                                 +instruction::def::PEK_OFF,
                                 +registers::def::sp,
                                 0,
                                 +instruction::def::PEK_OFF,
                                 +registers::def::sp,
                                 1,
                                 +instruction::def::ADD,
                                 +instruction::def::PEK_OFF,
                                 +registers::def::sp,
                                 2,
                                 +instruction::def::PEK_OFF,
                                 +registers::def::sp,
                                 1,
                                 +instruction::def::ADD,
                                 +instruction::def::POP_REG,
                                 +registers::def::ret,
                                 +instruction::def::RET};

    std::string dissassembly = generator.disassemble();


    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, LetStatement_DivisionExpression) {
    // setup
    std::string code(R"(let x = 10 + 20
                        let y = 3
                        return x / y)");
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = {+instruction::def::PSH_LIT,
                                 0,
                                 10,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 20,
                                 +instruction::def::ADD,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 3,
                                 +instruction::def::PEK_OFF,
                                 +registers::def::sp,
                                 0,
                                 +instruction::def::PEK_OFF,
                                 +registers::def::sp,
                                 1,
                                 +instruction::def::DIV,
                                 +instruction::def::POP_REG,
                                 +registers::def::ret,
                                 +instruction::def::RET};

    std::string dissassembly = generator.disassemble();


    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, Return_Equals) {
    // setup
    std::string code("return 5 == 2");
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = {+instruction::def::PSH_LIT,
                                 0,
                                 5,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 2,
                                 +instruction::def::CMP,
                                 +registers::def::sp,
                                 +instruction::def::MOV,
                                 +registers::def::ret,
                                 +registers::def::imm,
                                 +instruction::def::RET};

    std::string dissassembly = generator.disassemble();

    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, Return_EqualsIdentifer) {
    // setup
    std::string code(R"(let x = 10
                        return x == 10)");

    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = {+instruction::def::PSH_LIT,
                                 0,
                                 10,
                                 +instruction::def::PEK_OFF,
                                 +registers::def::sp,
                                 0,
                                 +instruction::def::PSH_LIT,
                                 0,
                                 10,
                                 +instruction::def::CMP,
                                 +registers::def::sp,
                                 +instruction::def::MOV,
                                 +registers::def::ret,
                                 +registers::def::imm,
                                 +instruction::def::RET};

    std::string dissassembly = generator.disassemble();

    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, IncIdentifierExpression) {
    // setup
    std::string code(R"(let x = 10
                        x++
                        return x)");
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = {+instruction::def::PSH_LIT,
                                 0,
                                 10,
                                 +instruction::def::INC,
                                 +registers::def::sp,
                                 0,
                                 +instruction::def::PEK_OFF,
                                 +registers::def::ret,
                                 0,
                                 +instruction::def::RET};

    std::string dissassembly = generator.disassemble();

    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}
