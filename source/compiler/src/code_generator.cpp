#include "code_generator.hpp"

#include <fmt/core.h>

#include "ast.hpp"

void CodeGenerator::generateCode()
{
    
    for (ASTBaseNode* node : m_program->readStatements())
    {       
        switch (node->readType())
        {
            case ASTNodeType::BINARY_EXPRESSION:
            {
                ASTBinaryExpressionNode* binaryNode = static_cast<ASTBinaryExpressionNode*>(node);
                generateBinaryExpression(binaryNode);
                break;
            }
            default:
            {
                // fmt::print("Unknown node type\n");
                break;
            }
        }
    }
}

void CodeGenerator::generateBinaryExpression(const ASTBinaryExpressionNode* node)
{    
    switch (node->readLeft()->readType())
    {
        case ASTNodeType::NUMERIC_LITERAL:
        {
            const auto* left = static_cast<const ASTNumericLiteralNode*>(node->readLeft());

            // 4 byte integer
            int32_t value = left->readValue();
            m_bytecode.push(static_cast<uint8_t>((value >> 24) & 0xFF));
            m_bytecode.push(static_cast<uint8_t>((value >> 16) & 0xFF));
            m_bytecode.push(static_cast<uint8_t>((value >> 8) & 0xFF));
            m_bytecode.push(static_cast<uint8_t>(value & 0xFF));
            break;
        }
        default:
        {
            // fmt::print("Unknown node type\n");
            break;
        }
    }

    m_bytecode.push(static_cast<uint8_t>(node->readOperator()));
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
