#include "processing_unit.hpp"
#include "instructions.hpp"
#include <shared_defines.hpp>

ProcessingUnit::ProcessingUnit()
{
    m_registries = m_memory.allocate(static_cast<uint16_t>(Register::reg_cnt));
}

void ProcessingUnit::load_program(uint8_t* program, uint16_t size)
{
    uint16_t addrs = m_memory.load(program, static_cast<uint16_t>(size));    
    m_registries[+Register::pc] = addrs;
    m_registries[+Register::bp] = addrs;
    if (size % 2 != 0)
        size += 1;

    uint16_t stackAddrs = static_cast<uint16_t>(addrs + size);
    m_registries[+Register::sp] = stackAddrs; 
    m_registries[+Register::fp] = stackAddrs;

}

void ProcessingUnit::execute()
{
    instruction::def instr = instruction::def::RET;
    ExecutionContext context(m_registries, m_memory.getMemory());
    uint16_t& pc = m_registries[+Register::pc];
    do {
        instr = static_cast<instruction::def>(pc);
        instruction::handlers[instr](context);
        pc++;
    }
    while (instr != instruction::def::RET);
}