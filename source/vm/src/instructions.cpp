#include "instructions.hpp"
#include "processing_unit.hpp"
#include <functional>

template<typename T, size_t N = sizeof(T)>
T read(uint8_t* bytecode, uint16_t& pc)
{
    uint8_t value[N] {0};    
    for (size_t i = 0; i < N; i++)
    {
        value[i] |= bytecode[++pc];
    }

    return *reinterpret_cast<T*>(value);    
}

int16_t instruction::read_word(uint8_t* bytecode, uint16_t& pc)
{
    int16_t value = 0;
    value = (value << 8) | bytecode[++pc];
    value = (value << 8) | bytecode[++pc];
    return value;
}

void instruction::write_int16(uint8_t* bytecode, uint16_t& pc, int16_t value)
{
    bytecode[pc++] = value & u8(0xFF);
    bytecode[pc++] = (value >> 8) & u8(0xFF);
}

int16_t instruction::pop_helper(ExecutionContext& context)
{
    uint16_t& sp = context.registry[+Register::sp]; 
    int16_t value = 0;
    value = (value << 8) | context.bytecode[--sp];
    value = (value << 8) | context.bytecode[--sp];
    return value;
}

int16_t instruction::peek_helper(uint8_t* bytecode, uint16_t sp)
{
    int16_t value = 0;
    value = (value << 8) | bytecode[--sp];
    value = (value << 8) | bytecode[--sp];
    return value;
}

void instruction::push_helper(ExecutionContext& context, int16_t value)
{
    uint16_t& sp = context.registry[+Register::sp];
    write_int16(context.bytecode, sp, value);
}

/*
* The binary_stack_expression function is a helper function that takes a lambda function that
* represents the binary operation. Assumes that rhs will be ontop of stack and lhs underneath. 
* pushes back the result onto the stack */
void binary_stack_expression(ExecutionContext& context, std::function<int16_t(int16_t, int16_t)> op)
{
    // b will be the top of the stack
    int16_t b = instruction::pop_helper(context);

    // a will be the next value on the stack
    int16_t a = instruction::pop_helper(context);

    // push the result back onto the stack
    instruction::push_helper(context, op(a, b));
}

void instruction::add_handler(ExecutionContext& context)
{
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a + b; });
}

void instruction::sub_handler(ExecutionContext& context)
{
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a - b; });
}

void instruction::mul_handler(ExecutionContext& context)
{
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a * b; });
}

void instruction::div_handler(ExecutionContext& context)
{
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a / b; });
}

void instruction::peek_handler(ExecutionContext& context)
{
    uint16_t& pc = context.registry[+Register::pc];
    uint8_t reg = context.bytecode[++pc];
    uint16_t sp = context.registry[+Register::sp];
    int16_t value = peek_helper(context.bytecode, sp); // peek top of stack.    
    context.registry[reg] = static_cast<uint16_t>(value);
}

void instruction::push_handler(ExecutionContext& context)
{
    
}

void instruction::push_literal_handler(ExecutionContext& context)
{
    uint16_t& programCnt = context.registry[+Register::pc];    
    int16_t value = read_word(context.bytecode, programCnt);   
    instruction::push_helper(context, value);
}

void instruction::return_handler(ExecutionContext& context)
{
    context.return_value = pop_helper(context);
    
    uint16_t& pc = context.registry[+Register::pc];
    uint16_t& fp = context.registry[+Register::fp];
    uint16_t& sp = context.registry[+Register::sp];
    pc = fp -1; // setting the pc to the end of the program
    sp = fp;    
}

struct peek_offset_instrction
{
    uint8_t reg;
    uint8_t offset;
};

void instruction::peek_offset_handler(ExecutionContext& context)
{
    auto peek = read<peek_offset_instrction>(context.bytecode, context.registry[+Register::pc]);    
    uint16_t sp = context.registry[+Register::fp] + peek.offset;
    int16_t value = instruction::read_word(context.bytecode, sp);
    context.registry[peek.reg] = value;
}