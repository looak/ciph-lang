#pragma once
#include <cstdint>
#include <string>

class Disassembler
{
public:
    Disassembler(const uint8_t* program, size_t size) : m_program(program), m_size(size) {}
    ~Disassembler() = default;

    std::string disassemble() const;

private:
    std::string disassembleInstruction(size_t& program_count) const;
    std::string dissassembleNumericLiteral(size_t& program_count) const;

    const uint8_t* m_program;
    size_t m_size;
};