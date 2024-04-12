#include <gtest/gtest.h>

#include "ast.hpp"
#include "code_generator.hpp"
#include "lexar_defines.hpp"

class CodeGeneratorTestFixture : public ::testing::Test
{
public:
    void SetUp() override
    {
     
    }

    ASTProgramNode m_program;

};


/*
* Expect byte code array to contain 4 bytes representing the integer 42424242 */
TEST_F(CodeGeneratorTestFixture, NumericLiteral_ExpectArray)
{
    // setup
    ASTNumericLiteralNode* node = new ASTNumericLiteralNode(-558907665); // 0xDEAFBEEF

    m_program.addStatement(node);

    CodeGenerator generator(&m_program);

    // do
    generator.generateCode();

    // validate
    EXPECT_EQ(generator.readByteCode().size(), 4);
    EXPECT_EQ(generator.readByteCode()[0], 0xDE);
    EXPECT_EQ(generator.readByteCode()[1], 0xAF);
    EXPECT_EQ(generator.readByteCode()[2], 0xBE);
    EXPECT_EQ(generator.readByteCode()[3], 0xEF);
}
