#pragma once

#include <cstdint>
#include <string>
#include <vector>

class ASTBaseNode;
class ASTBinaryExpressionNode;
class ASTNumericLiteralNode;
class ASTProgramNode;
class ASTReturnNode;

class CodeGenerator
{
public:
	explicit CodeGenerator(const ASTProgramNode* program) : m_program(program) {}
	~CodeGenerator() = default;

	void generateCode();


    std::string outputBytecode();
    std::string disassemble() const;

private:
    void generateProgram(const ASTProgramNode* node);
    void generateReturnStatement(const ASTReturnNode* node);
    void generateOperator(const ASTBinaryExpressionNode* node);
    void generateExpression(const ASTBaseNode* node);
    void generateBinaryExpression(const ASTBinaryExpressionNode* node);
    void generateNumericLiteral(const ASTNumericLiteralNode* node);

    const ASTProgramNode* m_program;

    std::vector<uint8_t> m_bytecode;
    std::string m_resultBytecode;
};