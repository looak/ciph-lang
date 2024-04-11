#include "instructions.hpp"

void instruction::add_handler(ExecutionContext& context)
{
    auto& stack = context.stack;

    auto a = stack.top();
    stack.pop();
    context.sp--;

    auto b = stack.top();
    stack.pop();
    context.sp--;

    stack.push(a + b);
}

void instruction::push_handler(ExecutionContext& context)
{
    auto& stack = context.stack;
    context.pc++;
    stack.push(context.bytecode[context.pc]);
    context.sp++;
}