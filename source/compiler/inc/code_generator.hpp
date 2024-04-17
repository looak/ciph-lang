#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

class ASTBaseNode;
class ASTBinaryExpressionNode;
class ASTLetNode;
class ASTIdentifierNode;
class ASTNumericLiteralNode;
class ASTProgramNode;
class ASTReturnNode;


class CodeGenerator
{
public:
	explicit CodeGenerator(const ASTProgramNode* program) : m_program(program) {}
	~CodeGenerator() = default;

	void generateCode();

    const std::pair<uint8_t*, size_t> readRawBytecode() const;
    std::string outputBytecode();
    std::string disassemble() const;

private:
    void generateProgram(const ASTProgramNode* node);
    
    // expressions
    void generateExpression(const ASTBaseNode* node);
    void generateBinaryExpression(const ASTBinaryExpressionNode* node);
    void generateNumericLiteral(const ASTNumericLiteralNode* node);    
    void generateOperator(const ASTBinaryExpressionNode* node);
    void generateIdentifier(const ASTIdentifierNode* node);

    // statements
    void generateReturnStatement(const ASTReturnNode* node);
    void generateLetStatement(const ASTLetNode* node);
    
    // helper
    void peek();
    void push();
    void pop();
    void encode(int16_t value);
   
    // program
    uint16_t m_stackSize = 0;
    std::unordered_map<std::string, uint16_t> m_identifiers = {};

    const ASTProgramNode* m_program = nullptr;
    std::vector<uint8_t> m_bytecode = {};
    std::string m_resultBytecode = "";
};