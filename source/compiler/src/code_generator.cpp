#include <ciph/compiler/code_generator.hpp>

#include <utility>

#include <fmt/core.h>

#include <ciph/shared/disassembler.hpp>
#include <ciph/shared/shared_defines.hpp>

#include <ciph/compiler/ast.hpp>

using namespace ciph;

std::string
CodeGenerator::disassemble() const {
    Disassembler disassembler(&m_bytecode[0], m_bytecode.size());
    return disassembler.disassemble();
}

std::vector<Diagnostic>
CodeGenerator::generateCode() {
    m_bytecode.clear();
    m_errors.clear();

    emit(instruction::def::JMP);
    encode(0x0000); // placeholder for address, patched below

    // Main starts right after the jump. This only holds while function bodies
    // are not generated; once they are, main starts after them and this
    // address has to be set when the first top-level statement is emitted.
    uint16_t main_addrs = static_cast<uint16_t>(m_bytecode.size());
    m_functions.emplace("main", FunctionContext{"main", main_addrs});

    generateProgram(m_program);

    emit(instruction::def::HALT);

    // patch jump to main
    // jmp is at position 0, address is at position 1
    patch(1, m_functions["main"].address);

    resolveUnresolvedCalls();

    return m_errors;
}

void
CodeGenerator::generateProgram(const ASTProgramNode* node) {
    generateScope(node);
}

void
CodeGenerator::generateFunction(const ASTFunctionNode* node) {
    // Function bodies are not generated yet. Say so instead of emitting a
    // program whose calls have nowhere to go.
    report(ErrorCode::CODEGEN_ERROR_UNSUPPORTED, positionOf(node),
           fmt::format("function '{}': functions cannot be compiled yet", node->readName()));
}

void
CodeGenerator::generateScope(const ASTScopeNode* node) {
    for (ASTBaseNode* statement : node->readStatements()) {
        switch (statement->readType()) {
            case ASTNodeType::RETURN: {
                const auto* returnNode = static_cast<const ASTReturnNode*>(statement);
                generateReturnStatement(returnNode);
                break;
            }
            case ASTNodeType::LET: {
                const auto* letNode = static_cast<const ASTLetNode*>(statement);
                generateLetStatement(letNode);
                break;
            }
            case ASTNodeType::WHILE: {
                const auto* whileNode = static_cast<const ASTWhileNode*>(statement);
                generateWhileStatement(whileNode);
                break;
            }
            case ASTNodeType::FUNCTION: {
                const auto* functionNode = static_cast<const ASTFunctionNode*>(statement);
                generateFunction(functionNode);
                break;
            }
            default: {
                generateExpression(statement, registers::def::sp);
                break;
            }
        }
    }
}

void
CodeGenerator::generateExpression(const ASTBaseNode* node, registers::def reg) {
    switch (node->readType()) {
        case ASTNodeType::NUMERIC_LITERAL: {
            const auto* numericNode = static_cast<const ASTNumericLiteralNode*>(node);
            generateNumericLiteral(numericNode);
            break;
        }
        case ASTNodeType::COMPARISON_EXPRESSION: {
            const auto* comparisonNode = static_cast<const ASTComparisonExpressionNode*>(node);
            generateComparisonExpression(comparisonNode, registers::def::sp);
            m_bytecode.push_back(static_cast<uint8_t>(instruction::def::MOV));
            encodeRegister(reg);
            encodeRegister(registers::def::imm); // result of compare is stored in imm
            break;
        }
        case ASTNodeType::BINARY_EXPRESSION: {
            const auto* binaryNode = static_cast<const ASTBinaryExpressionNode*>(node);
            generateBinaryExpression(binaryNode, reg);
            break;
        }
        case ASTNodeType::IDENTIFIER: {
            const auto* identifierNode = static_cast<const ASTIdentifierNode*>(node);
            generateIdentifier(identifierNode, reg);
            break;
        }
        case ASTNodeType::CALL_EXPRESSION: {
            const auto* callNode = static_cast<const ASTCallNode*>(node);
            generateCall(callNode);
            break;
        }
        default: {
            report(ErrorCode::CODEGEN_ERROR_INTERNAL, positionOf(node),
                   fmt::format("no code generation for node type {}", static_cast<int>(node->readType())));
            break;
        }
    }
}

void
CodeGenerator::generateCall(const ASTCallNode* callNode) {
    if (auto function = m_functions.find(callNode->readFunctionName()); function != m_functions.end()) {
        m_bytecode.push_back(static_cast<uint8_t>(instruction::def::JMP));
        encode(function->second.address);
    }
    else {
        // Look the name up as a view; it is only copied into an owning key
        // the first time the function is called.
        auto unresolved = m_unresolvedCalls.find(callNode->readFunctionName());
        if (unresolved == m_unresolvedCalls.end()) {
            unresolved = m_unresolvedCalls.emplace(callNode->readFunctionName(), std::vector<uint8_t>{}).first;
        }
        emit(instruction::def::JMP);
        unresolved->second.push_back(static_cast<uint8_t>(m_bytecode.size())); // position to patch later
        encode(0x0000); // placeholder
    }
}

void
CodeGenerator::generateComparisonExpression(const ASTComparisonExpressionNode* node, registers::def regA,
                                            std::optional<registers::def> regB) {
    // giving stack pointer, which indicates we're pushing the result to the stack
    generateExpression(node->readLeft(), registers::def::sp);
    generateExpression(node->readRight(), registers::def::sp);
    generateCompareOperator(node, regA, regB);
}

void
CodeGenerator::generateBinaryExpression(const ASTBinaryExpressionNode* node, std::optional<registers::def> reg) {
    // giving stack pointer, which indicates we're pushing the result to the stack
    generateExpression(node->readLeft(), registers::def::sp);
    generateExpression(node->readRight(), registers::def::sp);
    generateOperator(node, reg);
}

void
CodeGenerator::generateReturnStatement(const ASTReturnNode* node) {
    generateExpression(node->readExpression(), registers::def::ret);
    m_bytecode.push_back(static_cast<uint8_t>(instruction::def::RET));
}

void
CodeGenerator::generateWhileStatement(const ASTWhileNode* node) {
    uint16_t start = static_cast<uint16_t>(m_bytecode.size());

    generateScope(node);

    generateComparisonExpression(node->readCondition(), registers::def::sp);

    uint16_t end = static_cast<uint16_t>(m_bytecode.size());
    switch (node->readCondition()->readOperator())
    {
    case OperatorType::LESS_THAN:
        m_bytecode.push_back(static_cast<uint8_t>(instruction::def::JLT));
        // since JLT is 3 bytes long, we need to add that to the end.
        uint16_t offset = (end + 3) - start;
        encode(offset);
        break;
    }
}

void
CodeGenerator::generateLetStatement(const ASTLetNode* node) {
    if (m_identifiers.find(node->readIdentifier()) == m_identifiers.end()) {
        if (m_stackSize >= UINT8_MAX) {
            report(ErrorCode::CODEGEN_ERROR_TOO_MANY_VALUES, positionOf(node),
                   fmt::format("no room left for '{}': a variable's stack offset has to fit in one byte",
                               node->readIdentifier()));
            return;
        }
        m_identifiers.emplace(  std::string(node->readIdentifier()),
                                IdentifierContext(node->readIdentifier(), static_cast<uint8_t>(m_stackSize)));
        generateExpression(node->readExpression(), registers::def::sp);
    }
    else {
        // The analyzer lets sibling blocks reuse a name, but variables here
        // are still kept in one list for the whole program.
        report(ErrorCode::CODEGEN_ERROR_UNSUPPORTED, positionOf(node),
               fmt::format("'{}' is declared in more than one block, which cannot be compiled yet",
                           node->readIdentifier()));
    }
}

void
CodeGenerator::generateIdentifier(const ASTIdentifierNode* node, registers::def reg) {
    if (node->readOperator() != nullptr) {
        if (auto identifier = m_identifiers.find(node->readName()); identifier != m_identifiers.end()) {
            const auto* op = static_cast<const ASTIncDecNode*>(node->readOperator());
            instruction::def opInstruction = op->readIsIncrement() ? instruction::def::INC : instruction::def::DEC;
            m_bytecode.push_back(static_cast<uint8_t>(opInstruction));
            m_bytecode.push_back(+registers::def::sp);
            m_bytecode.push_back(identifier->second.offset);
            return;
        }
        else {
            report(ErrorCode::CODEGEN_ERROR_INTERNAL, positionOf(node),
                   fmt::format("no stack slot for '{}'", node->readName()));
            return;
        }
    }
    if (auto identifier = m_identifiers.find(node->readName()); identifier != m_identifiers.end()) {
        if (peek_offset(identifier->second.offset, reg) == false)
            m_registers[+reg].value = std::make_optional(identifier->second);
    }
    else {
        report(ErrorCode::CODEGEN_ERROR_INTERNAL, positionOf(node),
               fmt::format("no stack slot for '{}'", node->readName()));
    }
}

void
CodeGenerator::encodeOperator(const ASTBinaryExpressionNode* node) {
    switch (node->readOperator()) {
        case OperatorType::ADDITION: {
            m_bytecode.push_back(static_cast<uint8_t>(instruction::def::ADD));
            break;
        }
        case OperatorType::SUBTRACTION: {
            m_bytecode.push_back(static_cast<uint8_t>(instruction::def::SUB));
            break;
        }
        case OperatorType::MULTIPLICATION: {
            m_bytecode.push_back(static_cast<uint8_t>(instruction::def::MUL));
            break;
        }
        case OperatorType::DIVISION: {
            m_bytecode.push_back(static_cast<uint8_t>(instruction::def::DIV));
            break;
        }
        default: {
            report(ErrorCode::CODEGEN_ERROR_INTERNAL, positionOf(node),
                   fmt::format("no instruction for operator {}", static_cast<int>(node->readOperator())));
            break;
        }
    }
}

void
CodeGenerator::generateOperatorReg( const ASTBinaryExpressionNode* node, registers::def regA,
                                    [[maybe_unused]] std::optional<registers::def> regB) {
    encodeOperator(node);

    if (regA == registers::def::ret) {
        m_bytecode.push_back(static_cast<uint8_t>(instruction::def::POP_REG));
        m_bytecode.push_back(+registers::def::ret);
    }
    m_stackSize -= 2; // poped twice
}

void
CodeGenerator::generateOperator(const ASTBinaryExpressionNode* node, std::optional<registers::def> regA,
                                std::optional<registers::def> regB) {
    if (regA.has_value() && regA.value() != registers::def::sp) {
        generateOperatorReg(node, regA.value(), regB);
    }
    else {
        encodeOperator(node);
        m_stackSize--; // poped twice & pushed once
    }
}

void
CodeGenerator::generateCompareOperator(const ASTComparisonExpressionNode* node, registers::def regA,
                                        std::optional<registers::def> regB) {
    m_bytecode.push_back(static_cast<uint8_t>(instruction::def::CMP));
    encodeRegister(regA);
    if (regB.has_value()) {
        encodeRegister(regB.value());
    }
    else if (regA != registers::def::sp) {
        report(ErrorCode::CODEGEN_ERROR_INTERNAL, positionOf(node),
               "a compare between registers needs two registers, got one");
    }
    else {
        m_stackSize -= 2; // poped twice
    }
}

void
CodeGenerator::generateNumericLiteral(const ASTNumericLiteralNode* node) {
    emit(instruction::def::PSH_LIT);
    m_stackSize++; // one value pushed
    // 2 byte integer
    int16_t value = node->readValue();
    encode(value);
}

std::string
CodeGenerator::outputBytecode() {
    std::string result{};

    for (auto byte : m_bytecode) {
        result += fmt::format("{:02X} ", byte);
    }

    // cache the result
    m_resultBytecode = result;
    return result;
}

const std::pair<uint8_t*, size_t>
CodeGenerator::readRawBytecode() const {
    uint8_t* result = new uint8_t[m_bytecode.size()];
    std::copy(m_bytecode.begin(), m_bytecode.end(), result);
    return {result, m_bytecode.size()};
}

void
CodeGenerator::encode(int16_t value) {
    m_bytecode.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    m_bytecode.push_back(static_cast<uint8_t>(value & 0xFF));
}

void
CodeGenerator::encodeRegister(registers::def reg) {
    m_bytecode.push_back(+reg);
}

void CodeGenerator::patch(uint16_t position, uint16_t byte) { 
    m_bytecode[position] = static_cast<uint8_t>((byte >> 8) & 0xFF);
    m_bytecode[position + 1] = static_cast<uint8_t>(byte & 0xFF);
}

bool
CodeGenerator::peek_offset(uint8_t offset, registers::def reg) {
    m_bytecode.push_back(static_cast<uint8_t>(instruction::def::PEK_OFF));
    encodeRegister(reg);
    m_bytecode.push_back(offset);

    return reg == registers::def::sp;
}

void CodeGenerator::resolveUnresolvedCalls() {
    for (const auto& [functionName, positions] : m_unresolvedCalls) {
        if (auto function = m_functions.find(functionName); function != m_functions.end()) {
            for (const auto& pos : positions) {
                patch(pos, function->second.address);
            }
        }
        else if (m_errors.empty()) {
            // With an earlier error, such as the function not being compiled,
            // the missing target is already explained.
            report(ErrorCode::CODEGEN_ERROR_INTERNAL, Position{0, 0},
                   fmt::format("call to '{}' has no address to jump to", functionName));
        }
    }
}

Position
CodeGenerator::positionOf(const ASTBaseNode* node) const {
    const SourceRange range = node->readSourceRange();
    const uint32_t token = range.focusToken();
    if (!range.hasTokens() || token >= m_tokens.size()) {
        return Position{0, 0};
    }
    return m_tokens[token].readPosition();
}

void
CodeGenerator::report(ErrorCode code, Position position, std::string message) {
    m_errors.push_back(Diagnostic{.code = code, .position = position, .additionalInfo = std::move(message)});
}