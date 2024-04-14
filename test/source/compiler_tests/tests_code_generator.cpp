#include <gtest/gtest.h>
#include <fmt/core.h>

#include "shared_defines.hpp"

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

uint8_t operator+(Instructions instr)
{
    return static_cast<uint8_t>(instr);
}

bool compareBytecode(const uint8_t* expected, const uint8_t* actual, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        if (expected[i] != actual[i])
        {
            return false;
        }
    }
    return true;
}

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
    uint8_t expectedProgram[] = { +Instructions::PUSH, 0xDE, 0xAF, 0xBE, 0xEF };
    uint32_t expectedSize = 5;
    
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
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
    uint8_t expectedProgram[] = { +Instructions::PUSH, 0xDE, 0xAF, 0xBE, 0xEF,
                                  +Instructions::PUSH, 0xCA, 0xFE, 0xBA, 0xBE,
                                  +Instructions::ADD };

    uint32_t expectedSize = 11;

    auto [actualProgram, actualSize] = generator.readRawBytecode();    

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
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
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { +Instructions::PUSH, 0, 0, 0, 2,
                                  +Instructions::PUSH, 0, 0, 0, 2,
                                  +Instructions::MUL,
                                  +Instructions::PUSH, 0, 0, 0, 3,
                                  +Instructions::ADD };
    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, GeneratingOrderOfOperation_PaarenthasesBeforeMul)
{
    // setup    
    std::string code("2 * (2 + 3)");    
    Parser parser(code);
    CodeGenerator generator(static_cast<ASTProgramNode*>(parser.parse()));

    // do
    generator.generateCode();    
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { +Instructions::PUSH, 0, 0, 0, 2,
                                  +Instructions::PUSH, 0, 0, 0, 2,
                                  +Instructions::PUSH, 0, 0, 0, 3,
                                  +Instructions::ADD,
                                  +Instructions::MUL };
    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));    
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
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { +Instructions::PUSH, 0, 0, 0, 2,
                                  +Instructions::PUSH, 0, 0, 0, 2,
                                  +Instructions::PUSH, 0, 0, 0, 3,
                                  +Instructions::ADD,
                                  +Instructions::MUL,
                                  +Instructions::RET };
    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, LetStatement_Expression)
{
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
    uint8_t expectedProgram[] = { +Instructions::PUSH, 0, 0, 0, 2,
                                  +Instructions::PUSH, 0, 0, 0, 2,
                                  +Instructions::PUSH, 0, 0, 0, 3,
                                  +Instructions::ADD,
                                  +Instructions::MUL,
                                  +Instructions::PEEK, 0, 0, 0, 0,
                                  +Instructions::RET };

    std::string dissassembly = generator.disassemble();

    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, LetStatement_MultipleVariables)
{
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
    uint8_t expectedProgram[] = {   +Instructions::PUSH, 0, 0, 0, 2,
                                    +Instructions::PUSH, 0, 0, 0, 3,
                                    +Instructions::PEEK, 0, 0, 0, 0,
                                    +Instructions::PEEK, 0, 0, 0, 1,
                                    +Instructions::ADD,
                                    +Instructions::PEEK, 0, 0, 0, 2,                                  
                                    +Instructions::PEEK, 0, 0, 0, 1,
                                    +Instructions::ADD,
                                    +Instructions::RET};

    std::string dissassembly = generator.disassemble();


    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, LetStatement_DivisionExpression)
{
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
    uint8_t expectedProgram[] = {   +Instructions::PUSH, 0, 0, 0, 10,
                                    +Instructions::PUSH, 0, 0, 0, 20,                                    
                                    +Instructions::ADD,
                                    +Instructions::PUSH, 0, 0, 0, 3,
                                    +Instructions::PEEK, 0, 0, 0, 0,                                  
                                    +Instructions::PEEK, 0, 0, 0, 1,
                                    +Instructions::DIV,
                                    +Instructions::RET};

    std::string dissassembly = generator.disassemble();


    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}
