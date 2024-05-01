#pragma once
#include <cstdint>
#include <string>
#include <map>

class Disassembler
{
public:
    Disassembler(const uint8_t* program, size_t size) : m_program(program), m_size(size) {}
    ~Disassembler() = default;

    std::string disassemble() const;
    std::string disassembleInstructionAt(size_t program_count) const;

    const std::map<size_t, std::string>& disassembledInstructions() const
    {
        return m_disassembledInstructions;
    }

private:
    std::string disassembleInstruction(size_t& program_count) const;
    std::string dissassembleNumericLiteral(size_t& program_count) const;
    std::string disassembleOffset(size_t& program_count) const;
    std::string dissassembleReg(size_t& program_count) const;
    std::string disassembleInstructionWithOptionalReg(size_t& program_count) const;

    mutable std::map<size_t, std::string> m_disassembledInstructions;
    const uint8_t* m_program;
    size_t m_size;
};