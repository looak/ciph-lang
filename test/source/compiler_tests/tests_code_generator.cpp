#include <gtest/gtest.h>
#include <fmt/core.h>

#include "ast.hpp"
#include "code_generator.hpp"
#include "lexar_defines.hpp"
#include "parser.hpp"

class CodeGeneratorTestFixture : public ::testing::Test
{
public:
    void SetUp() override
    {
     
    }

    ASTProgramNode m_program;
};

/*
* Expect byte code array to contain 4 bytes representing the integer 0xDEAFBEEF */
TEST_F(CodeGeneratorTestFixture, NumericLiteral_ExpectArray)
{
    // setup
    ASTNumericLiteralNode* node = new ASTNumericLiteralNode(-558907665); // 0xDEAFBEEF

    m_program.addStatement(node);

    CodeGenerator generator(&m_program);

    // do
    generator.generateCode();

    // validate
    std::string expected = "01 DE AF BE EF ";
    std::string actual = generator.outputBytecode();

    EXPECT_EQ(expected, actual);
}

TEST_F(CodeGeneratorTestFixture, BinaryExpression_ExpectArray)
{
    // setup
    ASTNumericLiteralNode* left = new ASTNumericLiteralNode(-558907665); // 0xDEAFBEEF
    ASTNumericLiteralNode* right = new ASTNumericLiteralNode(0xCAFEBABE); // 0xCAFEBABE

    ASTBinaryExpressionNode* node = new ASTBinaryExpressionNode(left, right, OperatorType::ADDITION);

    m_program.addStatement(node);

    CodeGenerator generator(&m_program);

    // do
    generator.generateCode();

    // validate
    std::string expected = "01 DE AF BE EF 01 CA FE BA BE 03 ";
    std::string actual = generator.outputBytecode();

    EXPECT_EQ(expected, actual);
}

TEST_F(CodeGeneratorTestFixture, GeneratingOrderOfOperation_MulBeforeAdd)
{
    // setup    
    std::string code("2 * 2 + 3");
    //std::string code("(1 + 2 * 3 + 2) / 3");
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();

    // validate
    std::string expected = "01 00 00 00 02 01 00 00 00 02 05 01 00 00 00 03 03 ";
    EXPECT_EQ(expected, actual);    
}

TEST_F(CodeGeneratorTestFixture, GeneratingOrderOfOperation_PaarenthasesBeforeMul)
{
    // setup    
    std::string code("2 * (2 + 3)");    
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();

    // validate
    std::string expected = "01 00 00 00 02 01 00 00 00 02 01 00 00 00 03 03 05 ";
    EXPECT_EQ(expected, actual);

    fmt::print("Disassembled: \n{}\n", generator.disassemble());    
}

TEST_F(CodeGeneratorTestFixture, ReturnStatment_Expression)
{
    // setup    
    std::string code("return 2 * (2 + 3)");
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();

    // validate
    std::string expected = "01 00 00 00 02 01 00 00 00 02 01 00 00 00 03 03 05 08 ";
    EXPECT_EQ(expected, actual);

    fmt::print("Disassembled: \n{}\n", generator.disassemble());    
}