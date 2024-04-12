#pragma once

#include <queue>
#include <cstdint>
#include <string>

class ASTBaseNode;
class ASTProgramNode;
class ASTBinaryExpressionNode;
class ASTNumericLiteralNode;

class CodeGenerator
{
public:
	explicit CodeGenerator(const ASTProgramNode* program) : m_program(program) {}
	~CodeGenerator() = default;

	void generateCode();

    std::string outputBytecode();

private:
    void generateOperator(const ASTBinaryExpressionNode* node);
    void generateExpression(const ASTBaseNode* node);
    void generateBinaryExpression(const ASTBinaryExpressionNode* node);
    void generateNumericLiteral(const ASTNumericLiteralNode* node);

    const ASTProgramNode* m_program;

    std::queue<uint8_t> m_bytecode;
    std::string m_resultBytecode;
};