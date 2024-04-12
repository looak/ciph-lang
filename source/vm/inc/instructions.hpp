#pragma once

#include <shared_defines.hpp>

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


static handler set[] = {
	nullptr, // 0
	&push_handler,
	nullptr, // pop_handler
	&add_handler,
	&sub_handler,
	&mul_handler,
	&div_handler

};


} // namespace instruction