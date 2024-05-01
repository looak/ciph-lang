#include "code_generator.hpp"

#include <fmt/core.h>

#include <disassembler.hpp>
#include <shared_defines.hpp>
#include "ast.hpp"

std::string CodeGenerator::disassemble() const
{
    Disassembler disassembler(&m_bytecode[0], m_bytecode.size());
    return disassembler.disassemble();
}

void CodeGenerator::generateCode()
{    
    generateProgram(m_program);
}

void CodeGenerator::generateProgram(const ASTProgramNode* node)
{
    for (ASTBaseNode* statement : node->readStatements())
    {
        switch (statement->readType())
        {
            case ASTNodeType::RETURN:
            {
                const auto* returnNode = static_cast<const ASTReturnNode*>(statement);
                generateReturnStatement(returnNode);
                break;
            }
            case ASTNodeType::LET:
            {
                const auto* letNode = static_cast<const ASTLetNode*>(statement);
                generateLetStatement(letNode);
                break;
            }
            default:
            {
                generateExpression(statement, registers::def::sp);
                break;
            }
        }
    }
}

void CodeGenerator::generateExpression(const ASTBaseNode* node, registers::def reg)
{
    switch (node->readType())
    {
        case ASTNodeType::NUMERIC_LITERAL:
        {
            const auto* numericNode = static_cast<const ASTNumericLiteralNode*>(node);
            generateNumericLiteral(numericNode);
            break;
        }
        case ASTNodeType::COMPARISON_EXPRESSION:
        {
            const auto* comparisonNode = static_cast<const ASTComparisonExpressionNode*>(node);
            generateComparisonExpression(comparisonNode, registers::def::sp);
            m_bytecode.push_back(static_cast<uint8_t>(instruction::def::MOV));
            encodeRegister(reg);
            encodeRegister(registers::def::imm); // result of compare is stored in imm
            break;
        }
        case ASTNodeType::BINARY_EXPRESSION:
        {
            const auto* binaryNode = static_cast<const ASTBinaryExpressionNode*>(node);
            generateBinaryExpression(binaryNode, reg);
            break;
        }
        case ASTNodeType::IDENTIFIER:
        {
            const auto* identifierNode = static_cast<const ASTIdentifierNode*>(node);
            generateIdentifier(identifierNode, reg);
            break;
        }
        default:
        {
            fmt::print("Unknown node type\n");
            break;
        }
    }
}

void CodeGenerator::generateComparisonExpression(const ASTComparisonExpressionNode* node, registers::def regA, std::optional<registers::def> regB)
{   
    // giving stack pointer, which indicates we're pushing the result to the stack
    generateExpression(node->readLeft(), registers::def::sp);
    generateExpression(node->readRight(), registers::def::sp);    
    generateCompareOperator(node, regA, regB);
}

void CodeGenerator::generateBinaryExpression(const ASTBinaryExpressionNode* node, std::optional<registers::def> reg)
{   
    // giving stack pointer, which indicates we're pushing the result to the stack
    generateExpression(node->readLeft(), registers::def::sp);
    generateExpression(node->readRight(), registers::def::sp);    
    generateOperator(node, reg);
}

void CodeGenerator::generateReturnStatement(const ASTReturnNode* node)
{
    generateExpression(node->readExpression(), registers::def::ret);
    m_bytecode.push_back(static_cast<uint8_t>(instruction::def::RET));
}

void CodeGenerator::generateLetStatement(const ASTLetNode* node)
{
    if (m_identifiers.find(node->readIdentifier()) == m_identifiers.end())
    {
        if (m_stackSize >= UINT8_MAX)
        {
            fmt::print("Stack overflow\n");
            return;
        }
        m_identifiers.emplace(node->readIdentifier(), IdentifierContext(node->readIdentifier(), static_cast<uint8_t>(m_stackSize)));
        generateExpression(node->readExpression(), registers::def::sp);
    }
    else
    {
        fmt::print("Identifier already exists\n");
    }
}

void CodeGenerator::generateIdentifier(const ASTIdentifierNode* node, registers::def reg)
{
    if (node->readOperator() != nullptr)
    {
        if (auto identifier = m_identifiers.find(node->readName()); identifier != m_identifiers.end())
        {
            const auto* op = static_cast<const ASTIncDecNode*>(node->readOperator());
            instruction::def opInstruction = op->readIsIncrement() ? instruction::def::INC : instruction::def::DEC;
            m_bytecode.push_back(static_cast<uint8_t>(opInstruction));
            m_bytecode.push_back(+registers::def::sp);
            m_bytecode.push_back(identifier->second.offset);
            return;
        }
        else
        {
            fmt::print("Identifier not found\n");
            return;
        }    
    }
    if (auto identifier = m_identifiers.find(node->readName()); identifier != m_identifiers.end())
    {
        if (peek_offset(identifier->second.offset, reg) == false)
            m_registers[+reg].value = std::make_optional(identifier->second);
    }
    else
    {
        fmt::print("Identifier not found\n");
    }
}

void CodeGenerator::encodeOperator(OperatorType op)
{
    switch (op)
    {
        case OperatorType::ADDITION:
        {
            m_bytecode.push_back(static_cast<uint8_t>(instruction::def::ADD));
            break;
        }
        case OperatorType::SUBTRACTION:
        {
            m_bytecode.push_back(static_cast<uint8_t>(instruction::def::SUB));
            break;
        }
        case OperatorType::MULTIPLICATION:
        {
            m_bytecode.push_back(static_cast<uint8_t>(instruction::def::MUL));
            break;
        }
        case OperatorType::DIVISION:
        {
            m_bytecode.push_back(static_cast<uint8_t>(instruction::def::DIV));
            break;
        }
        default:
        {
            fmt::print("Unknown operator\n");
            break;
        }
    }
}

void CodeGenerator::generateOperatorReg(const ASTBinaryExpressionNode* node, registers::def regA, std::optional<registers::def> regB)
{
    encodeOperator(node->readOperator());

    if (regA == registers::def::ret)
    {
        m_bytecode.push_back(static_cast<uint8_t>(instruction::def::POP_REG));
        m_bytecode.push_back(+registers::def::ret);
    }
    m_stackSize -= 2; // poped twice
}

void CodeGenerator::generateOperator(const ASTBinaryExpressionNode* node, std::optional<registers::def> regA, std::optional<registers::def> regB)
{
    if (regA.has_value() && regA.value() != registers::def::sp)
    {
        generateOperatorReg(node, regA.value(), regB);        
    }
    else
    {
        encodeOperator(node->readOperator());    
        m_stackSize--; // poped twice & pushed once
    }
}

void CodeGenerator::generateCompareOperator(const ASTComparisonExpressionNode* node, registers::def regA, std::optional<registers::def> regB)
{
    m_bytecode.push_back(static_cast<uint8_t>(instruction::def::CMP));
    encodeRegister(regA);
    if (regB.has_value())
    {
        encodeRegister(regB.value());
    }
    else
    if (regA != registers::def::sp)
    {
        fmt::print("Error: compare operator\n");    
    }
    else
    {
        m_stackSize -= 2; // poped twice    
    }
}

void CodeGenerator::generateNumericLiteral(const ASTNumericLiteralNode* node)
{    
    push(instruction::def::PSH_LIT);
    // 4 byte integer
    int16_t value = node->readValue();
    encode(value);
}

std::string CodeGenerator::outputBytecode()
{    
    std::string result{};

    for (auto byte : m_bytecode) {
        result += fmt::format("{:02X} ", byte);
    }

    // cache the result
    m_resultBytecode = result;
    return result;
}

const std::pair<uint8_t*, size_t>
CodeGenerator::readRawBytecode() const
{
    uint8_t* result = new uint8_t[m_bytecode.size()];
    std::copy(m_bytecode.begin(), m_bytecode.end(), result);
    return {result, m_bytecode.size()};
}

void CodeGenerator::encode(int16_t value)
{
    m_bytecode.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    m_bytecode.push_back(static_cast<uint8_t>(value & 0xFF));
}

void CodeGenerator::encodeRegister(registers::def reg)
{
    m_bytecode.push_back(+reg);
}

bool CodeGenerator::peek_offset(uint8_t offset, registers::def reg)
{
    m_bytecode.push_back(static_cast<uint8_t>(instruction::def::PEK_OFF));
    encodeRegister(reg); 
    m_bytecode.push_back(offset);

    return reg == registers::def::sp;
}

void CodeGenerator::push(instruction::def pushType)
{
    m_stackSize++;
    m_bytecode.push_back(static_cast<uint8_t>(pushType)); 
}