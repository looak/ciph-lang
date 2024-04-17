#include "processing_unit.hpp"
#include "instructions.hpp"
#include <shared_defines.hpp>

ProcessingUnit::ProcessingUnit() {
    m_reg_memory = m_memory.allocate(static_cast<uint16_t>(Register::reg_cnt));
    m_registers.set(m_reg_memory);    
}

void ProcessingUnit::load_program(uint8_t* program, uint16_t size)
{
    uint16_t addrs = m_memory.load(program, static_cast<uint16_t>(size));    
    m_reg_memory[+Register::pc] = addrs;
    m_reg_memory[+Register::bp] = addrs;
    if (size % 2 != 0)
        size += 1;

    uint16_t stackAddrs = static_cast<uint16_t>(addrs + size);
    m_reg_memory[+Register::sp] = stackAddrs; 
    m_reg_memory[+Register::fp] = stackAddrs;

}

int16_t ProcessingUnit::execute()
{
    instruction::def instr = instruction::def::RET;
    ExecutionContext context(m_reg_memory, m_memory.getMemory());
    uint16_t& pc = m_reg_memory[+Register::pc];
    do {        
        instr = static_cast<instruction::def>(context.bytecode[pc]);
        instruction::handlers[instr](context);
        pc++;
    }
    while (instr != instruction::def::RET);

    return context.return_value;
}