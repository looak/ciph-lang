#include "disassembler.hpp"
#include "shared_defines.hpp"
#include <fmt/core.h>

std::string Disassembler::disassembleInstruction(size_t& program_count) const
{
    Instructions i = static_cast<Instructions>(m_program[program_count]);
    std::string result = fmt::format("{} ", s_instructionMnemonics.at(i));
    switch (i)
    {
        case Instructions::PUSH:
            result += dissassembleNumericLiteral(program_count);
            //program_count++;
    }
    return result += "\n";

}
std::string Disassembler::dissassembleNumericLiteral(size_t& program_count) const
{
    int32_t value = 0;
    value = (value << 8) | m_program[++program_count];
    value = (value << 8) | m_program[++program_count];
    value = (value << 8) | m_program[++program_count];
    value = (value << 8) | m_program[++program_count];

    return fmt::format("{}", value);
}

std::string Disassembler::disassemble() const
{
    std::string disassembly{};

    for (size_t i = 0; i < m_size; i++)
    {
        disassembly += disassembleInstruction(i);
    }

    return disassembly;
}
