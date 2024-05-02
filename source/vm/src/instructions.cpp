#include "instructions.hpp"

#include <functional>

#include "processing_unit.hpp"

using namespace ciph;

template <typename T, size_t N = sizeof(T)>
T
read(uint8_t* bytecode, uint16_t& pc) {
    uint8_t value[N]{0};
    for (size_t i = 0; i < N; i++) {
        value[i] |= bytecode[++pc];
    }

    return *reinterpret_cast<T*>(value);
}

int16_t
instruction::stack_read_at_offset(uint8_t* bytecode, uint16_t& sp) {
    int16_t value = 0;
    value = (value << 8) | bytecode[--sp];
    value = (value << 8) | bytecode[--sp];
    return value;
}

int16_t
instruction::read_word(uint8_t* bytecode, uint16_t& pc) {
    int16_t value = 0;
    value = (value << 8) | bytecode[pc];
    value = (value << 8) | bytecode[++pc];
    return value;
}

void
instruction::write_int16(uint8_t* bytecode, uint16_t& pc, int16_t value) {
    bytecode[pc++] = static_cast<uint8_t>(value & u8(0xFF));
    bytecode[pc++] = static_cast<uint8_t>((value >> 8) & u8(0xFF));
}

int16_t
instruction::pop_helper(ExecutionContext& context) {
    uint16_t& sp = context.registry[+registers::def::sp];
    int16_t value = 0;
    value = (value << 8) | context.bytecode[--sp];
    value = (value << 8) | context.bytecode[--sp];
    return value;
}

int16_t
instruction::peek_helper(uint8_t* bytecode, uint16_t sp) {
    int16_t value = 0;
    value = (value << 8) | bytecode[--sp];
    value = (value << 8) | bytecode[--sp];
    return value;
}

void
instruction::push_helper(ExecutionContext& context, int16_t value) {
    uint16_t& sp = context.registry[+registers::def::sp];
    write_int16(context.bytecode, sp, value);
}

void
instruction::push_helper_reg(ExecutionContext& context, registers::def reg) {
    uint16_t& value = context.registry[+reg];
    push_helper(context, i16(value));
}

/*
 * The binary_stack_expression function is a helper function that takes a lambda function that
 * represents the binary operation. Assumes that rhs will be ontop of stack and lhs underneath.
 * pushes back the result onto the stack */
void
binary_stack_expression(ExecutionContext& context, std::function<int16_t(int16_t, int16_t)> op) {
    // b will be the top of the stack
    int16_t b = instruction::pop_helper(context);

    // a will be the next value on the stack
    int16_t a = instruction::pop_helper(context);

    // push the result back onto the stack
    instruction::push_helper(context, op(a, b));
}

void
instruction::add_handler(ExecutionContext& context) {
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a + b; });
}

void
instruction::sub_handler(ExecutionContext& context) {
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a - b; });
}

void
instruction::mul_handler(ExecutionContext& context) {
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a * b; });
}

void
instruction::div_handler(ExecutionContext& context) {
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a / b; });
}

void
instruction::peek_handler(ExecutionContext& context) {
    uint16_t& pc = context.registry[+registers::def::pc];
    uint8_t reg = context.bytecode[++pc];
    uint16_t sp = context.registry[+registers::def::sp];
    int16_t value = peek_helper(context.bytecode, sp); // peek top of stack.
    context.registry[reg] = static_cast<uint16_t>(value);
}

void
instruction::push_handler(ExecutionContext& context) {
    push_helper_reg(context, registers::def::imm);
}

void
instruction::push_reg_handler(ExecutionContext& context) {
    uint16_t& pc = context.registry[+registers::def::pc];
    uint8_t reg = context.bytecode[++pc];
    instruction::push_helper(context, i16(context.registry[reg]));
}

void
instruction::push_literal_handler(ExecutionContext& context) {
    uint16_t& programCnt = context.registry[+registers::def::pc];
    int16_t value = read_word(context.bytecode, ++programCnt);
    instruction::push_helper(context, value);
}

void
instruction::cmp_handler(ExecutionContext& context) {
    uint16_t& programCnt = context.registry[+registers::def::pc];
    uint8_t reg = context.bytecode[++programCnt];
    if (reg == +registers::def::sp) {
        int16_t b = pop_helper(context);
        int16_t a = pop_helper(context);
        context.registry[+registers::def::imm] = a - b;
    }
}

void
instruction::mov_handler(ExecutionContext& context) {
    uint16_t& programCnt = context.registry[+registers::def::pc];
    uint8_t regX = context.bytecode[++programCnt];
    uint8_t regY = context.bytecode[++programCnt];
    context.registry[regX] = context.registry[regY];
}

void
instruction::return_handler(ExecutionContext& context) {
    context.return_value = context.registry[+registers::def::ret];

    uint16_t& pc = context.registry[+registers::def::pc];
    uint16_t& fp = context.registry[+registers::def::fp];
    uint16_t& sp = context.registry[+registers::def::sp];
    pc = fp - 1; // setting the pc to the end of the program
    sp = fp;
}

struct peek_offset_instrction {
    uint8_t reg;
    uint8_t offset;
};

void
instruction::peek_offset_handler(ExecutionContext& context) {
    auto peek = read<peek_offset_instrction>(context.bytecode, context.registry[+registers::def::pc]);
    uint16_t sp = context.registry[+registers::def::fp] + (peek.offset * 2) + 2;
    int16_t value = instruction::stack_read_at_offset(context.bytecode, sp);

    if (peek.reg == +registers::def::sp)
        push_helper(context, value);
    else
        context.registry[peek.reg] = value;
}

void
instruction::pop_reg_handler(ExecutionContext& context) {
    uint16_t& pc = context.registry[+registers::def::pc];
    uint8_t reg = context.bytecode[++pc];
    context.registry[reg] = pop_helper(context);
}

void
instruction::inc_handler(ExecutionContext& context) {
    uint16_t& pc = context.registry[+registers::def::pc];
    uint8_t reg = context.bytecode[++pc];

    if (reg == +registers::def::sp) {
        uint8_t offset = context.bytecode[++pc];
        offset *= 2;
        offset += context.registry[+registers::def::fp] + 2;
        uint16_t offset16 = static_cast<uint16_t>(offset);
        int16_t value = stack_read_at_offset(context.bytecode, offset16);
        value++;
        write_int16(context.bytecode, offset16, value);
    }
    else {
        context.registry[reg]++;
    }
}
void
instruction::dec_handler(ExecutionContext& context) {
    uint16_t& pc = context.registry[+registers::def::pc];
    uint8_t reg = context.bytecode[++pc];

    if (reg == +registers::def::sp) {
        uint8_t offset = context.bytecode[++pc];
        offset *= 2;
        offset += context.registry[+registers::def::fp] + 2;
        uint16_t offset16 = static_cast<uint16_t>(offset);
        int16_t value = stack_read_at_offset(context.bytecode, offset16);
        value--;
        write_int16(context.bytecode, offset16, value);
    }
    else {
        context.registry[reg]--;
    }
}

void
instruction::jump_eq_handler(ExecutionContext& context) {
}
void
instruction::jump_nz_handler(ExecutionContext& context) {
}
void
instruction::jump_gt_handler(ExecutionContext& context) {
}

void
instruction::jump_lt_handler(ExecutionContext& context) {
    uint16_t& pc = context.registry[+registers::def::pc];
    int16_t value = read_word(context.bytecode, ++pc);
    int16_t result = i16(context.registry[+registers::def::imm]);
    if (result < 0) {
        pc -= value;
    }
}