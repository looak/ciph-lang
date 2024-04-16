#include "instructions.hpp"
#include "processing_unit.hpp"
#include <functional>

int16_t read_int16(uint8_t* bytecode, uint16_t& pc)
{
    int16_t value = 0;
    value = (value << 8) | bytecode[++pc];
    value = (value << 8) | bytecode[++pc];
    return value;
}

void write_int16(uint8_t* bytecode, uint16_t& pc, int16_t value)
{
    bytecode[pc++] = (value >> 8) & 0xFF;
    bytecode[pc++] = value & 0xFF;
}

/*
* The binary_stack_expression function is a helper function that takes a lambda function that
* represents the binary operation. Assumes that rhs will be ontop of stack and lhs underneath. 
* pushes back the result onto the stack */
void binary_stack_expression(ExecutionContext& context, std::function<int32_t(int32_t, int32_t)> op)
{
    // b will be the top of the stack
    uint16_t& sp = context.registry[+Register::sp];
    sp -= 2; // reduce stack ptr to read first value on stack
    uint8_t* stackPtr = context.bytecode + sp;
    auto b = read_int16(stackPtr, sp);

    // reduce stack ptr
    sp -= 2;

    // a will be the next value on the stack
    stackPtr = context.bytecode + sp;
    auto a = read_int16(stackPtr, sp);
    sp -= 2;

    // push the result back onto the stack
    stackPtr = context.bytecode + sp;
    auto result = op(a, b);
    write_int16(stackPtr, sp, result);
}

void instruction::add_handler(ExecutionContext& context)
{
    binary_stack_expression(context, [](int32_t a, int32_t b) { return a + b; });
}

void instruction::sub_handler(ExecutionContext& context)
{
    binary_stack_expression(context, [](int32_t a, int32_t b) { return a - b; });
}

void instruction::mul_handler(ExecutionContext& context)
{
    binary_stack_expression(context, [](int32_t a, int32_t b) { return a * b; });
}

void instruction::div_handler(ExecutionContext& context)
{
    binary_stack_expression(context, [](int32_t a, int32_t b) { return a / b; });
}

void instruction::peek_handler(ExecutionContext& context)
{

    // int32_t offset = 0;
    // //for (int i = 0; i < 4; i++)
    // {
    //     offset = (offset << 8) | context.bytecode[++context.pc];      
    //     offset = (offset << 8) | context.bytecode[++context.pc];      
    //     offset = (offset << 8) | context.bytecode[++context.pc];      
    //     offset = (offset << 8) | context.bytecode[++context.pc];      
    // }

    // std::vector<int32_t>& stack = context.stack;
    // auto value = stack[static_cast<size_t>(offset)];
    // stack.push_back(value);
    // context.sp++;
}

void instruction::push_handler(ExecutionContext& context)
{
    
}


void instruction::push_literal_handler(ExecutionContext& context)
{
    uint16_t& programCnt = context.registry[+Register::pc];
    uint16_t& sp = context.registry[+Register::sp];
    int16_t value = read_int16(context.bytecode, ++programCnt);

    uint8_t* stackPtr = context.bytecode + sp;
    write_int16(stackPtr, sp, value);
}

void instruction::return_handler(ExecutionContext& context)
{
    // context.return_value = context.stack.back();
    // context.stack.pop_back();
    // context.sp--;
    // context.pc = static_cast<uint32_t>(context.program_size); // end the program
    
    // Clear the stack
    // context.stack.clear();
    // context.sp = 0;
}