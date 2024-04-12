#include "instructions.hpp"

#include <functional>

void binary_expression(ExecutionContext& context, std::function<int32_t(int32_t, int32_t)> op)
{
    auto& stack = context.stack;

    // b will be the top of the stack
    auto b = stack.top();
    stack.pop();
    context.sp--;

    auto a = stack.top();
    stack.pop();
    context.sp--;

    stack.push(op(a, b));
    context.sp++;
}

void instruction::add_handler(ExecutionContext& context)
{
    binary_expression(context, [](int32_t a, int32_t b) { return a + b; });
}

void instruction::sub_handler(ExecutionContext& context)
{
    binary_expression(context, [](int32_t a, int32_t b) { return a - b; });
}

void instruction::mul_handler(ExecutionContext& context)
{
    binary_expression(context, [](int32_t a, int32_t b) { return a * b; });
}

void instruction::div_handler(ExecutionContext& context)
{
    binary_expression(context, [](int32_t a, int32_t b) { return a / b; });
}

void instruction::push_handler(ExecutionContext& context)
{
    auto& stack = context.stack;
    //context.pc++;
    // rebuild the integer from the 4 bytes
    int32_t value = 0;
    //for (int i = 0; i < 4; i++)
    {
        value = (value << 8) | context.bytecode[++context.pc];      
        value = (value << 8) | context.bytecode[++context.pc];      
        value = (value << 8) | context.bytecode[++context.pc];      
        value = (value << 8) | context.bytecode[++context.pc];      
    }    

    stack.push(value);
    context.sp++;
}