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
                generateExpression(statement);
                break;
            }
        }
    }
}

void CodeGenerator::generateExpression(const ASTBaseNode* node)
{
    switch (node->readType())
    {
        case ASTNodeType::NUMERIC_LITERAL:
        {
            const auto* numericNode = static_cast<const ASTNumericLiteralNode*>(node);
            generateNumericLiteral(numericNode);
            break;
        }
        case ASTNodeType::BINARY_EXPRESSION:
        {
            const auto* binaryNode = static_cast<const ASTBinaryExpressionNode*>(node);
            generateBinaryExpression(binaryNode);
            break;
        }
        case ASTNodeType::IDENTIFIER:
        {
            const auto* identifierNode = static_cast<const ASTIdentifierNode*>(node);
            generateIdentifier(identifierNode);
            break;
        }
        default:
        {
            fmt::print("Unknown node type\n");
            break;
        }
    }
}

void CodeGenerator::generateBinaryExpression(const ASTBinaryExpressionNode* node)
{         
    generateExpression(node->readLeft());
    generateExpression(node->readRight());
    generateOperator(node);
    m_stackSize--; // reduce by 2 add 1
}

void CodeGenerator::generateReturnStatement(const ASTReturnNode* node)
{
    generateExpression(node->readExpression());
    m_bytecode.push_back(static_cast<uint8_t>(instruction::def::RET));
}

void CodeGenerator::generateLetStatement(const ASTLetNode* node)
{
    if (m_identifiers.find(node->readIdentifier()) == m_identifiers.end())
    {
        m_identifiers[node->readIdentifier()] = m_stackSize;
        generateExpression(node->readExpression());
    }
    else
    {
        fmt::print("Identifier already exists\n");
    }
}

void CodeGenerator::generateIdentifier(const ASTIdentifierNode* node)
{
    if (m_identifiers.find(node->readName()) != m_identifiers.end())
    {
        peek();
        encode(static_cast<int16_t>(m_identifiers[node->readName()]));
    }
    else
    {
        fmt::print("Identifier not found\n");
    }
}

void CodeGenerator::generateOperator(const ASTBinaryExpressionNode* node)
{
    switch (node->readOperator())
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

void CodeGenerator::generateNumericLiteral(const ASTNumericLiteralNode* node)
{    
    push();
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

void CodeGenerator::peek()
{
    m_stackSize++;
    m_bytecode.push_back(static_cast<uint8_t>(instruction::def::PEK_OFF));
}

void CodeGenerator::push()
{
    m_stackSize++;
    m_bytecode.push_back(static_cast<uint8_t>(instruction::def::PSH_LIT)); 
}