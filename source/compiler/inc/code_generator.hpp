#pragma once

#include <shared_defines.hpp>
#include "lexar_defines.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <optional>
#include <variant>

class ASTBaseNode;
class ASTBinaryExpressionNode;
class ASTLetNode;
class ASTIdentifierNode;
class ASTNumericLiteralNode;
class ASTProgramNode;
class ASTReturnNode;

struct IdentifierContext {
    IdentifierContext(const std::string& name, uint8_t offset) : name(name), offset(offset) {}
    bool operator==(const std::string& _name) const { return this->name == _name; }

    std::string name = "";
    uint8_t offset = 0;
    uint8_t cur_register = 0xFF;
};

struct PointerContext {
    PointerContext(uint16_t address, uint8_t reg) : address(address), reg(reg) {}
    uint16_t address = 0;
    uint8_t reg = 0xFF;    
};

struct RegisterValue {
    std::optional<IdentifierContext> value;
};


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
    void generateExpression(const ASTBaseNode* node, registers::def reg);
    void generateBinaryExpression(const ASTBinaryExpressionNode* node, std::optional<registers::def> reg = std::nullopt);
    void generateNumericLiteral(const ASTNumericLiteralNode* node);    
    void generateOperator(const ASTBinaryExpressionNode* node, std::optional<registers::def> regA = std::nullopt, std::optional<registers::def> regB = std::nullopt);
    void generateOperatorReg(const ASTBinaryExpressionNode* node, registers::def regA, std::optional<registers::def> regB = std::nullopt);
    void generateIdentifier(const ASTIdentifierNode* node, registers::def reg = registers::def::imm);
    void generateIncDec(const ASTIdentifierNode* node, bool isIncrement);

    // statements
    void generateReturnStatement(const ASTReturnNode* node);
    void generateLetStatement(const ASTLetNode* node);
    
    
    /* @brief peek_offset
     * @param offset offset into the stack where the value is.
     * @param reg given register, if sp (stack pointer) is given, value gets pushed onto the stack.
     * @return true: the value was assigned to a register. false: the value was pushed to the stack.   */
    bool peek_offset(uint8_t offset, registers::def reg);

    void push(instruction::def pushType);
    void pop();
    void encode(int16_t value);
    void encodeRegister(registers::def reg);
    void encodeOperator(OperatorType op);

    std::array<RegisterValue, static_cast<int>(registers::def::reg_cnt)> m_registers;
   
    // program
    uint16_t m_stackSize = 0;
    std::unordered_map<std::string, IdentifierContext> m_identifiers;


    const ASTProgramNode* m_program = nullptr;
    std::vector<uint8_t> m_bytecode = {};
    std::string m_resultBytecode = "";
};