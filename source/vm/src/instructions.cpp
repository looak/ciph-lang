#include <ciph/vm/instructions.hpp>
#include <functional>
#include <ciph/vm/processing_unit.hpp>

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
    uint16_t& sp = context.reg(registers::def::sp);
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
    uint16_t& sp = context.reg(registers::def::sp);
    write_int16(context.bytecode, sp, value);
}

void
instruction::push_helper_reg(ExecutionContext& context, registers::def reg) {
    uint16_t& value = context.reg(reg);
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

template <>
void instruction::exec<instruction::def::ADD>(ExecutionContext& context) {
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a + b; });
}

template <>
void instruction::exec<instruction::def::SUB>(ExecutionContext& context) {
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a - b; });
}

template <>
void instruction::exec<instruction::def::MUL>(ExecutionContext& context) {
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a * b; });
}

template <>
void instruction::exec<instruction::def::DIV>(ExecutionContext& context) {
    binary_stack_expression(context, [](int16_t a, int16_t b) { return a / b; });
}

template <>
void instruction::exec<instruction::def::PEK_REG>(ExecutionContext& context) {
    uint16_t& pc = context.reg(registers::def::pc);
    auto reg = static_cast<registers::def>(context.bytecode[++pc]);
    uint16_t sp = context.reg(registers::def::sp);
    int16_t value = peek_helper(context.bytecode, sp); // peek top of stack.
    context.reg(reg) = static_cast<uint16_t>(value);
}

template <>
void instruction::exec<instruction::def::PSH>(ExecutionContext& context) {
    push_helper_reg(context, registers::def::imm);
}

template <>
void instruction::exec<instruction::def::PSH_REG>(ExecutionContext& context) {
    uint16_t& pc = context.reg(registers::def::pc);
    auto reg = static_cast<registers::def>(context.bytecode[++pc]);
    int16_t value = read_word(context.bytecode, ++pc);
    context.reg(reg) = value;
}

template <>
void instruction::exec<instruction::def::PSH_LIT>(ExecutionContext& context) {
    uint16_t& programCnt = context.reg(registers::def::pc);
    int16_t value = read_word(context.bytecode, ++programCnt);
    instruction::push_helper(context, value);
}

template <>
void instruction::exec<instruction::def::CMP>(ExecutionContext& context) {
    uint16_t& programCnt = context.reg(registers::def::pc);
    auto reg = static_cast<registers::def>(context.bytecode[++programCnt]);
    if (reg == registers::def::sp) {
        int16_t b = pop_helper(context);
        int16_t a = pop_helper(context);
        context.reg(registers::def::imm) = a - b;
    }
}

template <>
void instruction::exec<instruction::def::MOV>(ExecutionContext& context) {
    uint16_t& programCnt = context.reg(registers::def::pc);
    auto regX = static_cast<registers::def>(context.bytecode[++programCnt]);
    auto regY = static_cast<registers::def>(context.bytecode[++programCnt]);
    context.reg(regX) = context.reg(regY);
}

template <>
void instruction::exec<instruction::def::RET>(ExecutionContext& context) {
    context.return_value = context.reg(registers::def::ret);

    uint16_t& pc = context.reg(registers::def::pc);
    uint16_t& fp = context.reg(registers::def::fp);
    uint16_t& sp = context.reg(registers::def::sp);    
    sp = fp;
    fp = static_cast<uint16_t>(pop_helper(context));
    pc = static_cast<uint16_t>(pop_helper(context));
}

struct peek_offset_instrction {
    registers::def reg;
    int8_t offset;
};

template <>
void instruction::exec<instruction::def::PEK_OFF>(ExecutionContext& context) {
    auto peek = read<peek_offset_instrction>(context.bytecode, context.reg(registers::def::pc));
    uint16_t sp = context.reg(registers::def::fp) + (peek.offset * 2) + 2;
    int16_t value = instruction::stack_read_at_offset(context.bytecode, sp);
    

    if (peek.reg == registers::def::sp)
        push_helper(context, value);
    else
        context.reg(peek.reg) = value;
}

template <>
void instruction::exec<instruction::def::POP_REG>(ExecutionContext& context) {
    uint16_t& pc = context.reg(registers::def::pc);
    auto reg = static_cast<registers::def>(context.bytecode[++pc]);
    context.reg(reg) = pop_helper(context);
}

template <>
void instruction::exec<instruction::def::INC>(ExecutionContext& context) {
    uint16_t& pc = context.reg(registers::def::pc);
    auto reg = static_cast<registers::def>(context.bytecode[++pc]);

    if (reg == registers::def::sp) {
        uint8_t offset = context.bytecode[++pc];
        offset *= 2;
        offset = static_cast<uint8_t>(offset + context.reg(registers::def::fp) + 2);
        uint16_t offset16 = static_cast<uint16_t>(offset);
        int16_t value = stack_read_at_offset(context.bytecode, offset16);
        value++;
        write_int16(context.bytecode, offset16, value);
    }
    else {
        context.reg(reg)++;
    }
}
template <>
void instruction::exec<instruction::def::DEC>(ExecutionContext& context) {
    uint16_t& pc = context.reg(registers::def::pc);
    auto reg = static_cast<registers::def>(context.bytecode[++pc]);

    if (reg == registers::def::sp) {
        uint8_t offset = context.bytecode[++pc];
        offset *= 2;
        offset = static_cast<uint8_t>(offset + context.reg(registers::def::fp) + 2);
        uint16_t offset16 = static_cast<uint16_t>(offset);
        int16_t value = stack_read_at_offset(context.bytecode, offset16);
        value--;
        write_int16(context.bytecode, offset16, value);
    }
    else {
        context.reg(reg)--;
    }
}

// The conditional jumps all work the same way. They follow a CMP, which left
// (a - b) in imm, and read a 16-bit distance. When `taken` says yes to imm,
// pc moves that many bytes back, counted from the end of the jump instruction.
// They can only jump backwards, which is all a loop needs.
static void
jump_back_if(ExecutionContext& context, bool (*taken)(int16_t)) {
    uint16_t& pc = context.reg(registers::def::pc);
    int16_t distance = instruction::read_word(context.bytecode, ++pc);
    if (taken(i16(context.reg(registers::def::imm)))) {
        pc -= distance;
    }
}

template <>
void instruction::exec<instruction::def::JEQ>(ExecutionContext& context) {
    jump_back_if(context, [](int16_t difference) { return difference == 0; });
}

template <>
void instruction::exec<instruction::def::JNZ>(ExecutionContext& context) {
    jump_back_if(context, [](int16_t difference) { return difference != 0; });
}

template <>
void instruction::exec<instruction::def::JGT>(ExecutionContext& context) {
    jump_back_if(context, [](int16_t difference) { return difference > 0; });
}

template <>
void instruction::exec<instruction::def::JLT>(ExecutionContext& context) {
    jump_back_if(context, [](int16_t difference) { return difference < 0; });
}

template <>
void instruction::exec<instruction::def::JMP>(ExecutionContext& context) {
    uint16_t& pc = context.reg(registers::def::pc);
    uint16_t address = static_cast<uint16_t>(read_word(context.bytecode, ++pc));

    // The address counts from the start of the program, which is loaded at bp.
    // The execute loop increments pc after every handler, so land one byte
    // before the target.
    pc = static_cast<uint16_t>(context.reg(registers::def::bp) + address - 1);
}

template <>
void instruction::exec<instruction::def::CALL>(ExecutionContext& context) {
    uint16_t& pc = context.reg(registers::def::pc);
    uint16_t& fp = context.reg(registers::def::fp);
    uint16_t& sp = context.reg(registers::def::sp);
    uint16_t address = read_word(context.bytecode, ++pc);
    push_helper(context, pc);
    push_helper(context, fp);
    fp = sp;
    pc = static_cast<uint16_t>(context.reg(registers::def::bp) + address - 1);
}

template <>
void instruction::exec<instruction::def::HALT>(ExecutionContext&) {
    // Nothing to do: the execute loop stops when it sees HALT. Unlike RET it
    // leaves the return value and the stack untouched.
}

template <>
void instruction::exec<instruction::def::NEG>(ExecutionContext& context) {    
    int16_t value = pop_helper(context);
    push_helper(context, -value);
}
