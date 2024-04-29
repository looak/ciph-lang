#include "disassembler.hpp"
#include "shared_defines.hpp"
#include <fmt/core.h>

std::string Disassembler::disassembleInstruction(size_t& program_count) const
{
    auto instr = static_cast<instruction::def>(m_program[program_count]);
    std::string result = fmt::format("{} ", instruction::mnemonics.at(instr));
    switch (instr)
    {
        case instruction::def::PSH_LIT:
            result += dissassembleNumericLiteral(program_count);
            break;
        case instruction::def::INC:
        case instruction::def::DEC:
        case instruction::def::PEK_OFF:
            result += dissassembleReg(program_count);
            result += " [sp + ";
            result += disassembleOffset(program_count);
            result += "]";
            break;
            
        case instruction::def::POP_REG:
        case instruction::def::PEK_REG:
            result += dissassembleReg(program_count);
            break;            
            //program_count++;
    }
    return result += "\n";

}
std::string Disassembler::dissassembleNumericLiteral(size_t& program_count) const
{
    int16_t value = 0;
    value = (value << 8) | m_program[++program_count];
    value = (value << 8) | m_program[++program_count];
    return fmt::format("{}", value);
}

std::string Disassembler::disassembleOffset(size_t& program_count) const
{    
    return fmt::format("{}", m_program[++program_count]);
}

std::string Disassembler::dissassembleReg(size_t& program_count) const
{
    return fmt::format("{}", registers::name.at(static_cast<registers::def>(m_program[++program_count])));
}

std::string Disassembler::disassemble() const
{
    std::string disassembly{};

    for (size_t i = 0; i < m_size; i++)
    {
        auto key = i;
        m_disassembledInstructions.emplace(key, disassembleInstruction(i));        
        disassembly += m_disassembledInstructions[key];
    }

    return disassembly;
}

std::string Disassembler::disassembleInstructionAt(size_t program_count) const
{
    return m_disassembledInstructions[program_count];
}