#pragma once

#include <string>
#include <vector>
#include <unordered_map>

enum class Instructions : uint8_t {
	NOP = 0x0,
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

const std::unordered_map<Instructions, std::string> s_instructionMnemonics = {
	{Instructions::NOP, "NOP"},
	{Instructions::PUSH, "PUSH"},
	{Instructions::POP, "POP"},
	{Instructions::ADD, "ADD"},
	{Instructions::SUB, "SUB"},
	{Instructions::MUL, "MUL"},
	{Instructions::DIV, "DIV"},
	{Instructions::MOD, "MOD"}
};