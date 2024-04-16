#pragma once

#include <shared_defines.hpp>
#include <unordered_map>
#include "execution_context.hpp"

namespace instruction {
typedef void (*handler)(ExecutionContext& context);

void push_handler(ExecutionContext& context);
void add_handler(ExecutionContext& context);
void sub_handler(ExecutionContext& context);
void mul_handler(ExecutionContext& context);
void div_handler(ExecutionContext& context);
void return_handler(ExecutionContext& context);
void peek_handler(ExecutionContext& context);

static std::unordered_map<def, handler> handlers = {
	{def::PSH, push_handler},
	{def::ADD, add_handler},
	{def::SUB, sub_handler},
	{def::MUL, mul_handler},
	{def::DIV, div_handler},
	{def::RET, return_handler},
	{def::PEK_REG, peek_handler}
}; // handlers
} // namespace instruction