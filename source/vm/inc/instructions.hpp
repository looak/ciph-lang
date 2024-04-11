#pragma once

#include "execution_context.hpp"

enum class Instructions : int32_t{
	PUSH,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	INC,
	DEC,
	ASSIGN,
	ADD_ASSIGN,
	SUB_ASSIGN,
	MUL_ASSIGN,
	DIV_ASSIGN,
	MOD_ASSIGN,
	LSHIFT,
	RSHIFT,
	LSHIFT_ASSIGN,
	RSHIFT_ASSIGN,
	AND,
	OR,
	XOR,
	AND_ASSIGN,
	OR_ASSIGN,
	XOR_ASSIGN,
	EQUAL,
	NOT_EQUAL,
	LESS,
	GREATER,
	LESS_EQUAL,
	GREATER_EQUAL,
	AND_AND,
	OR_OR,
	UNKNOWN
};

inline int operator+(Instructions i) {
	return static_cast<int>(i);
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