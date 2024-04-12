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


static handler set[] = {
	&push_handler,
	&add_handler
};


} // namespace instruction