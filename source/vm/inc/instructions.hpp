#pragma once

#include <shared_defines.hpp>
#include <unordered_map>

#include "execution_context.hpp"

inline uint8_t operator+(Instructions i) {
	return static_cast<uint8_t>(i);
}

namespace instruction {
typedef void (*handler)(ExecutionContext& context);

void push_handler(ExecutionContext& context);
void add_handler(ExecutionContext& context);
void sub_handler(ExecutionContext& context);
void mul_handler(ExecutionContext& context);
void div_handler(ExecutionContext& context);
void return_handler(ExecutionContext& context);
void peek_handler(ExecutionContext& context);


static std::unordered_map<Instructions, handler> handlers = {
	{Instructions::PUSH, &push_handler},
	{Instructions::PEEK, &peek_handler},
	{Instructions::ADD, &add_handler},
	{Instructions::SUB, &sub_handler},
	{Instructions::MUL, &mul_handler},
	{Instructions::DIV, &div_handler},
	{Instructions::RET, &return_handler}
};


} // namespace instruction