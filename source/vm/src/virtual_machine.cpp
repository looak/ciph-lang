#include "virtual_machine.hpp"
#include "instructions.hpp"

VirtualMachine::VirtualMachine(uint8_t* program, size_t size)
    : m_program(program), m_size(size)
{
}

int32_t VirtualMachine::execute()
{
    ExecutionContext context;
    context.bytecode = m_program;
    context.pc = 0;
    context.sp = 0;
    context.return_value = 1; // error
    context.program_size = m_size;

    run(context);

    return context.return_value;
}

void VirtualMachine::run(ExecutionContext& context)
{
    while (context.pc < m_size)
    {
        auto instruction = static_cast<Instructions>(context.bytecode[context.pc]);
        instruction::handlers[instruction](context);
        context.pc++;
    }
}