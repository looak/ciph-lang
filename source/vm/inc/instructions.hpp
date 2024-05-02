#pragma once

#include <shared_defines.hpp>
#include <unordered_map>
#include "execution_context.hpp"

namespace ciph {
namespace instruction {

int16_t stack_read_at_offset(uint8_t* bytecode, uint16_t& sp);
int16_t read_word(uint8_t* bytecode, uint16_t& pc);
void write_int16(uint8_t* bytecode, uint16_t& pc, int16_t value);
int16_t peek_helper(uint8_t* bytecode, uint16_t sp);
int16_t pop_helper(ExecutionContext& context);
void push_helper(ExecutionContext& context, int16_t value);
void push_helper_reg(ExecutionContext& context, registers::def reg);

void jmp_helper(ExecutionContext& context, int16_t address);

typedef void (*handler)(ExecutionContext& context);

void push_handler(ExecutionContext& context);
void push_literal_handler(ExecutionContext& context);
void push_reg_handler(ExecutionContext& context);

void add_handler(ExecutionContext& context);
void sub_handler(ExecutionContext& context);
void mul_handler(ExecutionContext& context);
void div_handler(ExecutionContext& context);
void return_handler(ExecutionContext& context);
void peek_handler(ExecutionContext& context);
void peek_offset_handler(ExecutionContext& context);
void pop_reg_handler(ExecutionContext& context);
void inc_handler(ExecutionContext& context);
void dec_handler(ExecutionContext& context);
void cmp_handler(ExecutionContext& context);
void mov_handler(ExecutionContext& context);
void jump_eq_handler(ExecutionContext& context);
void jump_nz_handler(ExecutionContext& context);
void jump_gt_handler(ExecutionContext& context);
void jump_lt_handler(ExecutionContext& context);

static std::unordered_map<def, handler> handlers = {
	{def::PSH, push_handler},
	{def::PSH_REG, push_reg_handler},
	{def::PSH_LIT, push_literal_handler},
	{def::ADD, add_handler},
	{def::SUB, sub_handler},
	{def::MUL, mul_handler},
	{def::DIV, div_handler},
	{def::POP_REG, pop_reg_handler},
	{def::PEK_REG, peek_handler},
	{def::PEK_OFF, peek_offset_handler},
	{def::RET, return_handler},
	{def::INC, inc_handler},
	{def::DEC, dec_handler},
	{def::CMP, cmp_handler},
	{def::MOV, mov_handler},
	{def::JEQ, jump_eq_handler},
	{def::JNZ, jump_nz_handler},
	{def::JGT, jump_gt_handler},
	{def::JLT, jump_lt_handler}

}; // handlers

} // namespace instruction
} // namespace ciph