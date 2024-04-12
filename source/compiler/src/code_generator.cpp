#include "code_generator.hpp"

#include <fmt/core.h>
#include <shared_defines.hpp>
#include "ast.hpp"


void CodeGenerator::generateCode()
{    
    for (ASTBaseNode* node : m_program->readStatements())
    {       
        generateExpression(node);
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
}

void CodeGenerator::generateOperator(const ASTBinaryExpressionNode* node)
{
    switch (node->readOperator())
    {
        case OperatorType::ADDITION:
        {
            m_bytecode.push(static_cast<uint8_t>(Instructions::ADD));
            break;
        }
        case OperatorType::SUBTRACTION:
        {
            m_bytecode.push(static_cast<uint8_t>(Instructions::ADD));
            break;
        }
        case OperatorType::MULTIPLICATION:
        {
            m_bytecode.push(static_cast<uint8_t>(Instructions::MUL));
            break;
        }
        case OperatorType::DIVISION:
        {
            m_bytecode.push(static_cast<uint8_t>(Instructions::DIV));
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
    // instruction
    m_bytecode.push(static_cast<uint8_t>(Instructions::PUSH));
    // 4 byte integer
    int32_t value = node->readValue();
    m_bytecode.push(static_cast<uint8_t>((value >> 24) & 0xFF));
    m_bytecode.push(static_cast<uint8_t>((value >> 16) & 0xFF));
    m_bytecode.push(static_cast<uint8_t>((value >> 8) & 0xFF));
    m_bytecode.push(static_cast<uint8_t>(value & 0xFF));
}

std::string CodeGenerator::outputBytecode()
{    
    std::string output;
    while (!m_bytecode.empty()) {
        output += fmt::format("{:02X} ", m_bytecode.front());
        m_bytecode.pop();
    }

    // cache the result
    m_resultBytecode = output;
    return output;
}
