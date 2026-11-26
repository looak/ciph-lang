#include <gtest/gtest.h>
#include <fmt/core.h>

#include <ciph/shared/shared_defines.hpp>

#include <ciph/compiler/ast.hpp>
#include <ciph/compiler/code_generator.hpp>
#include <ciph/compiler/lexer_defines.hpp>
#include <ciph/compiler/parser.hpp>

using namespace ciph;

class CodeGeneratorTestFixture : public ::testing::Test
{
public:
    void SetUp() override
    {
     
    }

    ASTProgramNode m_program{SourceRange{}}; // built by hand, so no source tokens
};

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

TEST_F(CodeGeneratorTestFixture, NumericLiteral_ExpectArray)
{
    // setup
    ASTNumericLiteralNode* node = new ASTNumericLiteralNode(SourceRange{}, 0x0eef);

    m_program.addStatement(node);

    CodeGenerator generator(&m_program);

    // do
    generator.generateCode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0x0E, 0xEF,
                                    +instruction::def::RET };
    uint32_t expectedSize = sizeof(expectedProgram);
    
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, BinaryExpression_ExpectArray)
{
    // setup
    ASTNumericLiteralNode* left = new ASTNumericLiteralNode(SourceRange{}, 0x0AFE);
    ASTNumericLiteralNode* right = new ASTNumericLiteralNode(SourceRange{}, 0x0ABE);

    ASTBinaryExpressionNode* node = new ASTBinaryExpressionNode(SourceRange{}, left, right, OperatorType::ADDITION);

    m_program.addStatement(node);

    CodeGenerator generator(&m_program);

    // do
    generator.generateCode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0x0A, 0xFE,
                                    +instruction::def::PSH_LIT, 0x0A, 0xBE,
                                    +instruction::def::ADD,
                                    +instruction::def::RET };

    uint32_t expectedSize = sizeof(expectedProgram);

    auto [actualProgram, actualSize] = generator.readRawBytecode();    

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, GeneratingOrderOfOperation_MulBeforeAdd)
{
    // setup    
    std::string code("return 2 * 2 + 3;");
    //std::string code("(1 + 2 * 3 + 2) / 3");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode);

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0, 2,
                                    +instruction::def::PSH_LIT, 0, 2,
                                    +instruction::def::MUL,
                                    +instruction::def::PSH_LIT, 0, 3,
                                    +instruction::def::ADD,
                                    +instruction::def::POP_REG, +registers::def::ret,
                                    +instruction::def::RET };
    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

// Parentheses before multiplication: `return 2 * (2 + 3);`
TEST_F(CodeGeneratorTestFixture, ReturnStatment_Expression)
{
    // setup    
    std::string code("return 2 * (2 + 3);");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode);

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0, 2,
                                    +instruction::def::PSH_LIT, 0, 2,
                                    +instruction::def::PSH_LIT, 0, 3,
                                    +instruction::def::ADD,
                                    +instruction::def::MUL,
                                    +instruction::def::POP_REG, +registers::def::ret,
                                    +instruction::def::RET };
    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, ReturnStatment_Expression_Simple)
{
    // setup    
    std::string code("return 2 + 3;");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode);

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0, 2,
                                    +instruction::def::PSH_LIT, 0, 3,
                                    +instruction::def::ADD,
                                    +instruction::def::POP_REG, +registers::def::ret,
                                    +instruction::def::RET };
    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, LetStatement_Expression)
{
    // setup    
    std::string code(R"(let x = 2 * (2 + 3);
                        return x;)");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode);

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0, 2,
                                    +instruction::def::PSH_LIT, 0, 2,
                                    +instruction::def::PSH_LIT, 0, 3,
                                    +instruction::def::ADD,
                                    +instruction::def::MUL,
                                    +instruction::def::PEK_OFF, +registers::def::ret, 0,
                                    +instruction::def::RET };

    std::string dissassembly = generator.disassemble();

    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, LetStatement_MultipleVariables)
{
    // setup    
    std::string code(R"(let x = 2;
                        let y = 3;
                        let z = x + y;
                        return z + y;)");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode);

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0, 2,
                                    +instruction::def::PSH_LIT, 0, 3,
                                    +instruction::def::PEK_OFF, +registers::def::sp, 0,
                                    +instruction::def::PEK_OFF, +registers::def::sp, 1,
                                    +instruction::def::ADD,
                                    +instruction::def::PEK_OFF, +registers::def::sp, 2,
                                    +instruction::def::PEK_OFF, +registers::def::sp, 1,
                                    +instruction::def::ADD,
                                    +instruction::def::POP_REG, +registers::def::ret,
                                    +instruction::def::RET };

    std::string dissassembly = generator.disassemble();


    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, LetStatement_DivisionExpression)
{
    // setup    
    std::string code(R"(let x = 10 + 20;
                        let y = 3;
                        return x / y;)");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode);

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0, 10,
                                    +instruction::def::PSH_LIT, 0, 20,
                                    +instruction::def::ADD,
                                    +instruction::def::PSH_LIT, 0, 3,
                                    +instruction::def::PEK_OFF, +registers::def::sp, 0,
                                    +instruction::def::PEK_OFF, +registers::def::sp, 1,
                                    +instruction::def::DIV,
                                    +instruction::def::POP_REG, +registers::def::ret,
                                    +instruction::def::RET };

    std::string dissassembly = generator.disassemble();


    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, Return_Equals)
{
    // setup    
    std::string code("return 5 == 2;");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode);

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0, 5,
                                    +instruction::def::PSH_LIT, 0, 2,
                                    +instruction::def::CMP, +registers::def::sp,
                                    +instruction::def::MOV, +registers::def::ret, +registers::def::imm,
                                    +instruction::def::RET };

    std::string dissassembly = generator.disassemble();

    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, Return_EqualsIdentifer)
{
    // setup    
    std::string code(R"(let x = 10;
                        return x == 10;)");
    
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode);

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0, 10,
                                    +instruction::def::PEK_OFF, +registers::def::sp, 0,
                                    +instruction::def::PSH_LIT, 0, 10,
                                    +instruction::def::CMP, +registers::def::sp,
                                    +instruction::def::MOV, +registers::def::ret, +registers::def::imm,
                                    +instruction::def::RET };

    std::string dissassembly = generator.disassemble();

    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, IncIdentifierExpression)
{
    // setup    
    std::string code(R"(let x = 10;
                        x++;
                        return x;)");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode);

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0, 10,
                                    +instruction::def::INC, +registers::def::sp, 0,
                                    +instruction::def::PEK_OFF, +registers::def::ret, 0,
                                    +instruction::def::RET };

    std::string dissassembly = generator.disassemble();

    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, WhileStatement) {
    // setup
    std::string code(
        R"(
			let i = 0;
			while (i < 5) {
				i++;
			}
			return i;)");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());

    CodeGenerator generator(programNode);

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = { // every program starts by calling main, and halts when main returns
                                    +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0, 0,
                                    +instruction::def::INC, +registers::def::sp, 0,
                                    +instruction::def::PEK_OFF, +registers::def::sp, 0,
                                    +instruction::def::PSH_LIT, 0, 5,
                                    +instruction::def::CMP, +registers::def::sp,
                                    +instruction::def::JLT, 0x00, 0x0E, // jump back fourteen bytes
                                    +instruction::def::PEK_OFF, +registers::def::ret, 0,
                                    +instruction::def::RET };

    std::string dissassembly = generator.disassemble();

    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}

TEST_F(CodeGeneratorTestFixture, WhileStatement_JumpMatchesTheComparison) {
    struct Case {
        const char* comparison;
        instruction::def jump;
    };
    const Case cases[] = {
        {"<", instruction::def::JLT},
        {">", instruction::def::JGT},
        {"==", instruction::def::JEQ},
        {"!=", instruction::def::JNZ},
    };

    for (const Case& c : cases) {
        std::string code = fmt::format("let i = 0; while (i {} 5) {{ i++; }} return i;", c.comparison);
        Parser parser(code);
        auto parser_result = parser.parse();
        auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
        CodeGenerator generator(programNode, parser.readTokens());

        EXPECT_TRUE(generator.generateCode().empty()) << c.comparison;
        auto [actualProgram, actualSize] = generator.readRawBytecode();

        // Same layout as the WhileStatement test; only the jump differs.
        uint8_t expectedProgram[] = { +instruction::def::CALL, 0x00, 0x04,
                                      +instruction::def::HALT,
                                      +instruction::def::PSH_LIT, 0, 0,
                                      +instruction::def::INC, +registers::def::sp, 0,
                                      +instruction::def::PEK_OFF, +registers::def::sp, 0,
                                      +instruction::def::PSH_LIT, 0, 5,
                                      +instruction::def::CMP, +registers::def::sp,
                                      +c.jump, 0x00, 0x0E,
                                      +instruction::def::PEK_OFF, +registers::def::ret, 0,
                                      +instruction::def::RET };

        ASSERT_EQ(sizeof(expectedProgram), actualSize) << c.comparison;
        EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize)) << c.comparison;
    }
}

TEST_F(CodeGeneratorTestFixture, ValidProgram_ReportsNoErrors) {
    std::string code("let x = 2; return x + 3;");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode, parser.readTokens());

    EXPECT_TRUE(generator.generateCode().empty());
}

TEST_F(CodeGeneratorTestFixture, Function_ReportedAsUnsupportedAtItsName) {
    std::string code("fn number() {\n"
                     "    return 5;\n"
                     "}\n"
                     "return number();");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode, parser.readTokens());

    std::vector<Diagnostic> errors = generator.generateCode();

    // One error for the function; the call that has nowhere to jump to is a
    // consequence of it and is not reported separately.
    ASSERT_EQ(errors.size(), 1u);
    EXPECT_EQ(errors[0].code, ErrorCode::CODEGEN_ERROR_UNSUPPORTED);
    EXPECT_EQ(errors[0].location.span.start.line, 1u);
    EXPECT_EQ(errors[0].location.span.start.column, 4u);
}

TEST_F(CodeGeneratorTestFixture, NameReusedInSiblingBlocks_ReportedAsUnsupported) {
    std::string code("let i = 0;\n"
                     "while (i < 1) { let a = 1; i++; }\n"
                     "while (i < 2) { let a = 2; i++; }\n"
                     "return i;");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode, parser.readTokens());

    std::vector<Diagnostic> errors = generator.generateCode();

    ASSERT_EQ(errors.size(), 1u);
    EXPECT_EQ(errors[0].code, ErrorCode::CODEGEN_ERROR_UNSUPPORTED);
    EXPECT_EQ(errors[0].location.span.start.line, 3u);
}

TEST_F(CodeGeneratorTestFixture, HandBuiltTree_ErrorsHaveZeroPosition) {
    // A use of a variable nothing declared: the analyzer would have caught
    // it, so here it is the generator's own error.
    m_program.addStatement(new ASTIdentifierNode(SourceRange{}, "ghost"));
    CodeGenerator generator(&m_program);

    std::vector<Diagnostic> errors = generator.generateCode();

    ASSERT_EQ(errors.size(), 1u);
    EXPECT_EQ(errors[0].code, ErrorCode::CODEGEN_ERROR_INTERNAL);
    EXPECT_EQ(errors[0].location.span.start.line, 0u);
    EXPECT_EQ(errors[0].location.span.start.column, 0u);
}

// DISABLED: placeholder. Function bodies are not generated yet, and the expected
// bytes below were copied from the while test. Re-enable with real expectations
// once function support is designed and built.
TEST_F(CodeGeneratorTestFixture, DISABLED_SimpleFunctionReturn) {
    // setup
    std::string code(
        R"(
			fn number() {
                return 5+5;
            }
            return number();)");
    Parser parser(code);
    auto parser_result = parser.parse();
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());

    CodeGenerator generator(programNode);

    // do
    generator.generateCode();
    std::string actual = generator.outputBytecode();
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    // validate
    uint8_t expectedProgram[] = {   +instruction::def::PSH_LIT, 0, 0,
                                    +instruction::def::INC, +registers::def::sp, 0,
                                    +instruction::def::PEK_OFF, +registers::def::sp, 0,
                                    +instruction::def::PSH_LIT, 0, 5,
                                    +instruction::def::CMP, +registers::def::sp,
                                    +instruction::def::JLT, 0x00, 0x0E, // jump back fourteen bytes
                                    +instruction::def::PEK_OFF, +registers::def::ret, 0,
                                    +instruction::def::RET};

    std::string dissassembly = generator.disassemble();

    uint32_t expectedSize = sizeof(expectedProgram);

    EXPECT_EQ(expectedSize, actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}
// A chain groups from the left: (10 - 4) - 3. Grouped from the right the
// bytecode would push all three values before the first SUB.
TEST_F(CodeGeneratorTestFixture, ChainedSubtraction_SubtractsLeftToRight)
{
    std::string code("return 10 - 4 - 3;");
    Parser parser(code);
    auto parser_result = parser.parse();
    ASSERT_TRUE(parser_result.has_value());
    auto programNode = static_cast<ASTProgramNode*>(parser_result.value());
    CodeGenerator generator(programNode, parser.readTokens());

    EXPECT_TRUE(generator.generateCode().empty());
    auto [actualProgram, actualSize] = generator.readRawBytecode();

    uint8_t expectedProgram[] = { +instruction::def::CALL, 0x00, 0x04,
                                    +instruction::def::HALT,
                                    +instruction::def::PSH_LIT, 0, 10,
                                    +instruction::def::PSH_LIT, 0, 4,
                                    +instruction::def::SUB,
                                    +instruction::def::PSH_LIT, 0, 3,
                                    +instruction::def::SUB,
                                    +instruction::def::POP_REG, +registers::def::ret,
                                    +instruction::def::RET };

    ASSERT_EQ(sizeof(expectedProgram), actualSize);
    EXPECT_TRUE(compareBytecode(expectedProgram, actualProgram, actualSize));
}
