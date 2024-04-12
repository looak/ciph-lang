#pragma once

#include <queue>
#include <cstdint>
#include <string>

class ASTProgramNode;
class ASTBinaryExpressionNode;

class CodeGenerator
{
public:
	explicit CodeGenerator(const ASTProgramNode* program) : m_program(program) {}
	~CodeGenerator() = default;

	void generateCode();

    std::string outputBytecode();

private:
    void generateBinaryExpression(const ASTBinaryExpressionNode* node);

    const ASTProgramNode* m_program;

    std::queue<uint8_t> m_bytecode;
    std::string m_resultBytecode;
};