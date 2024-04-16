#include "instructions.hpp"

#include <functional>

void binary_expression(ExecutionContext& context, std::function<int32_t(int32_t, int32_t)> op)
{
    auto& stack = context.stack;

    // b will be the top of the stack
    auto b = stack.back();
    stack.pop_back();
    context.sp--;

    auto a = stack.back();
    stack.pop_back();
    context.sp--;

    stack.push_back(op(a, b));
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

void instruction::peek_handler(ExecutionContext& context)
{

    int32_t offset = 0;
    //for (int i = 0; i < 4; i++)
    {
        offset = (offset << 8) | context.bytecode[++context.pc];      
        offset = (offset << 8) | context.bytecode[++context.pc];      
        offset = (offset << 8) | context.bytecode[++context.pc];      
        offset = (offset << 8) | context.bytecode[++context.pc];      
    }

    std::vector<int32_t>& stack = context.stack;
    auto value = stack[static_cast<size_t>(offset)];
    stack.push_back(value);
    context.sp++;
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

    stack.push_back(value);
    context.sp++;
}

void instruction::return_handler(ExecutionContext& context)
{
    context.return_value = context.stack.back();
    context.stack.pop_back();
    context.sp--;
    context.pc = static_cast<uint32_t>(context.program_size); // end the program
    
    // Clear the stack
    context.stack.clear();
    context.sp = 0;
}