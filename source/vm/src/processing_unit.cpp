#include "processing_unit.hpp"
#include "instructions.hpp"
#include <fmt/core.h>

ProcessingUnit::ProcessingUnit() {
    m_reg_memory = m_memory.allocate(static_cast<uint16_t>(registers::def::reg_cnt));
    m_registers.set(m_reg_memory);    
}

void ProcessingUnit::load_program(uint8_t* program, uint16_t size)
{
    uint16_t addrs = m_memory.load(program, static_cast<uint16_t>(size));    
    m_reg_memory[+registers::def::pc] = addrs;
    m_reg_memory[+registers::def::bp] = addrs;
    if (size % 2 != 0)
        size += 1;

    uint16_t stackAddrs = static_cast<uint16_t>(addrs + size);
    m_reg_memory[+registers::def::sp] = stackAddrs; 
    m_reg_memory[+registers::def::fp] = stackAddrs;

    m_context.bytecode = m_memory.getMemory();
    m_context.registry = m_reg_memory;
}

int16_t ProcessingUnit::execute()
{
    instruction::def instr = instruction::def::RET;
    
    uint16_t& pc = m_reg_memory[+registers::def::pc];
    do {        
        instr = static_cast<instruction::def>(m_context.bytecode[pc]);
        instruction::handlers[instr](m_context);        
        pc++;

    }
    while (instr != instruction::def::RET);

    return m_context.return_value;
}

bool ProcessingUnit::step()
{
    instruction::def instr = instruction::def::RET;    
    uint16_t& pc = m_reg_memory[+registers::def::pc];
    instr = static_cast<instruction::def>(m_context.bytecode[pc]);
    instruction::handlers[instr](m_context);
    pc++;

    return instr != instruction::def::RET;
}