#pragma once

#include <shared_defines.hpp>
#include <unordered_map>
#include "execution_context.hpp"


namespace instruction {
	
int16_t read_int16(uint8_t* bytecode, uint16_t& pc);
void write_int16(uint8_t* bytecode, uint16_t& pc, int16_t value);
int16_t pop_helper(ExecutionContext& context);
void push_helper(ExecutionContext& context, int16_t value);

typedef void (*handler)(ExecutionContext& context);

void push_handler(ExecutionContext& context);
void push_literal_handler(ExecutionContext& context);

void add_handler(ExecutionContext& context);
void sub_handler(ExecutionContext& context);
void mul_handler(ExecutionContext& context);
void div_handler(ExecutionContext& context);
void return_handler(ExecutionContext& context);
void peek_handler(ExecutionContext& context);

static std::unordered_map<def, handler> handlers = {
	{def::PSH, push_handler},
	{def::PSH_LIT, push_literal_handler},
	{def::ADD, add_handler},
	{def::SUB, sub_handler},
	{def::MUL, mul_handler},
	{def::DIV, div_handler},
	{def::RET, return_handler},
	{def::PEK_REG, peek_handler}
}; // handlers
} // namespace instruction