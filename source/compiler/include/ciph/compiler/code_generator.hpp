#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <ciph/shared/shared_defines.hpp>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <ciph/compiler/diagnostics.hpp>
#include <ciph/compiler/lexer.hpp>
#include <ciph/compiler/lexer_defines.hpp>

namespace ciph {

class ASTBaseNode;
class ASTBinaryExpressionNode;
class ASTComparisonExpressionNode;
class ASTLetNode;
class ASTIdentifierNode;
class ASTNumericLiteralNode;
class ASTProgramNode;
class ASTReturnNode;
class ASTScopeNode;
class ASTWhileNode;
class ASTCallNode;
class ASTFunctionNode;

struct IdentifierContext {
    IdentifierContext(std::string_view _name, uint8_t _offset)
        : name(_name)
        , offset(_offset) {}
    bool
    operator==(const std::string& _name) const {
        return this->name == _name;
    }

    std::string name = "";
    uint8_t offset = 0;
    uint8_t cur_register = 0xFF;
};

struct PointerContext {
    PointerContext(uint16_t address, uint8_t reg)
        : address(address)
        , reg(reg) {}
    uint16_t address = 0;
    uint8_t reg = 0xFF;
};

struct FunctionContext {
    std::string name = "";
    uint16_t address = 0;
};

struct RegisterValue {
    std::optional<IdentifierContext> value;
};


// Turns a checked program into bytecode.
//
// Runs after the semantic analyzer and trusts it: a name that is used has been
// declared. What it reports itself are things it cannot compile yet, limits it
// ran into, and its own mistakes.
class CodeGenerator {
public:
    // `tokens` is the parser's token list (Parser::readTokens()); errors use
    // it to point into the source. Without it, such as for a tree built by
    // hand, errors carry a zero position.
    explicit CodeGenerator(const ASTProgramNode* program, std::span<const Token> tokens = {})
        : m_program(program)
        , m_tokens(tokens) {}
    ~CodeGenerator() = default;

    // Generates the whole program and returns every error found. An empty
    // result means the bytecode is complete; otherwise it should not be run.
    std::vector<Diagnostic> generateCode();

    const std::pair<uint8_t*, size_t> readRawBytecode() const;
    std::string outputBytecode();
    std::string disassemble() const;

private:


    void generateProgram(const ASTProgramNode* node);
    void generateScope(const ASTScopeNode* node);
    void generateFunction(const ASTFunctionNode* node);

    // expressions
    void generateExpression(const ASTBaseNode* node);
    void generateExpression(const ASTBaseNode* node, registers::def reg);
    void generateComparisonExpression(  const ASTComparisonExpressionNode* node, registers::def regA,
                                        std::optional<registers::def> regB = std::nullopt);
    void generateBinaryExpression(  const ASTBinaryExpressionNode* node,
                                    std::optional<registers::def> reg = std::nullopt);
    void generateNumericLiteral(const ASTNumericLiteralNode* node, registers::def reg);
    void generateCall(const ASTCallNode* callNode);
    void generateOperator(  const ASTBinaryExpressionNode* node, std::optional<registers::def> regA = std::nullopt,
                            std::optional<registers::def> regB = std::nullopt);
    void generateOperatorReg(   const ASTBinaryExpressionNode* node, registers::def regA,
                                std::optional<registers::def> regB = std::nullopt);
    void generateCompareOperator(   const ASTComparisonExpressionNode* node, registers::def regA,
                                    std::optional<registers::def> regB = std::nullopt);
    void generateIdentifier(const ASTIdentifierNode* node, registers::def reg = registers::def::imm);

    // statements
    void generateReturnStatement(const ASTReturnNode* node);
    void generateLetStatement(const ASTLetNode* node);
    void generateWhileStatement(const ASTWhileNode* node);

    // functions
    void resolveUnresolvedCalls();


    /* @brief peek_offset
     * @param offset offset into the stack where the value is.
     * @param reg given register, if sp (stack pointer) is given, value gets pushed onto the stack.
     * @return true: the value was assigned to a register. false: the value was pushed to the stack.   */
    bool peek_offset(uint8_t offset, registers::def reg);
    
    void pop();
    void emit(instruction::def opCode) { m_bytecode.push_back(static_cast<uint8_t>(opCode)); }
    void patch(uint16_t position, uint16_t byte);
    void encode(int16_t value);
    void encodeRegister(registers::def reg);
    void encodeOperator(const ASTBinaryExpressionNode* node);

    void report(ErrorCode code, Location location, std::string message);

    std::array<RegisterValue, static_cast<int>(registers::def::reg_cnt)> m_registers;

    struct IdentifierHash {
        using is_transparent = void;
        std::size_t operator()(std::string_view key) const noexcept {
            return std::hash<std::string_view>{}(key);
        }
    };

    // program
    uint16_t m_stackSize = 0;
    std::unordered_map<std::string, IdentifierContext, IdentifierHash, std::equal_to<>> m_identifiers;
    std::unordered_map<uint16_t, PointerContext> m_pointers;
    std::unordered_map<std::string, FunctionContext, IdentifierHash, std::equal_to<>> m_functions;
    std::unordered_map<std::string, std::vector<uint8_t>, IdentifierHash, std::equal_to<>> m_unresolvedCalls;

    const ASTProgramNode* m_program = nullptr;
    std::span<const Token> m_tokens;
    std::vector<Diagnostic> m_errors;
    std::vector<uint8_t> m_bytecode = {};
    std::string m_resultBytecode = "";
};

} // namespace ciph