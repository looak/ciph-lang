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

    run(context);

    if (context.stack.empty())
        return 0;
        
    return context.stack.top();
}

void VirtualMachine::run(ExecutionContext& context)
{
    while (context.pc < m_size)
    {
        instruction::set[context.bytecode[context.pc]](context);
        context.pc++;
    }
}