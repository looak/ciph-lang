#pragma once

#include <string>
#include <vector>
#include <map>

#ifdef SHARED_LIB_EXPORTS
#define SHARED_API __declspec(dllexport)
#else
#define SHARED_API __declspec(dllimport)
#endif

enum class Instructions : uint8_t {
	UNKNOWN = 0x0,
	// stack
	PUSH = 0x1,
	POP,

	// arithmetic
	ADD,
	SUB,
	MUL,
	DIV,
	MOD
};

SHARED_API std::string libName();