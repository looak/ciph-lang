#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace instruction {

enum class def : uint8_t {
	PUT		=		0x01,	// Takes the memory at the location and puts it in imm register.
	PUT_REG	=		0x04, 	// Takes value of imm and puts it in given register.
	PUT_LIT	=		0x05,	// Move literal into imm register.
	MOV		=		0x02,   // Move data from register rY to register rX, if rY is unspecified moves rX into imm register.
	MOV_MEM	=		0x03,	// Move data from imm register into given Memory.

	// Arithmetic instructions
	ADD	 	=		0x06, 	// Pops two values from the stack and adds them together. Result is pushed onto stack.
	ADD_REG	=		0x07, 	// Takes values at registers, adds them together and puts result into rX. If rY is unspecified adds imm to rX.
	SUB	 	=		0x08, 	// Subtracts second value on stack with first and pushes difference onto stack.
	SUB_REG	=		0x09, 	// Subtracts rX with rY and puts difference into rX. If rY is unspecified uses imm.
	MUL		=		0x0A, 	// Pops two values from the stack and multiplies them together.
	MUL_REG	=		0x0B, 	// Takes values at registers, multiplies them together and puts product into rX. If rY is unspecified uses imm as multiplier, still stores product in rX.
	DIV	 	=		0x0C, 	// Pops divisor of the stack, then pops dividen of the stack. Puts quotient of the divison onto stack. Remainder is lost.
	DIV_REG	=		0x0D, 	// Uses register rX as dividen and rY as divisor. Quotient is put into rX. Remainder is lost. If rY is unspecified uses imm as divisor.
	
	// Stack instructions
	PSH	 	=		0x10, 	// Pushes data from imm register onto stack.
	PSH_REG	=		0x20, 	// Pushes data from reg onto stack.
	PSH_LIT	=		0x11, 	// Pushes literal onto stack.
	PSH_MEM	=		0x21, 	// Takes value at memory adress and pushes it onto stack.
	POP	 	=		0x30, 	// Pops top of stack to given imm register
	POP_REG	=		0x40, 	// Pops top of stack to given register.
	PEK_REG	=		0x50, 	// Copies top value of stack into given register
	
	// Control flow instructions
	JMP	 	=		0xC0, 	// Unconditionally jump to address.
	JNZ	 	=		0xC1, 	// Jump to address if imm is not zero.
	RET	 	=		0xC2, 	// Returns value in imm and terminates the program.
 
	// Other instructions
	NOP	 	=		0x00  	// No operation instruction, program counter should just tick pass this.
};

const std::unordered_map<def, std::string> mnemonics = {
	{def::PUT, "PUT"},
	{def::PUT_REG, "PUT"},
	{def::PUT_LIT, "PUT"},
	{def::MOV, "MOV"},
	{def::MOV_MEM, "MOV"},
	{def::ADD, "ADD"},
	{def::ADD_REG, "ADD"},
	{def::SUB, "SUB"},
	{def::SUB_REG, "SUB"},
	{def::MUL, "MUL"},
	{def::MUL_REG, "MUL"},
	{def::DIV, "DIV"},
	{def::DIV_REG, "DIV"},
	{def::PSH, "PSH"},
	{def::PSH_REG, "PSH"},
	{def::PSH_LIT, "PSH"},
	{def::PSH_MEM, "PSH"},
	{def::POP, "POP"},
	{def::POP_REG, "POP"},
	{def::PEK_REG, "PEK"},
	{def::JMP, "JMP"},
	{def::JNZ, "JNZ"},
	{def::RET, "RET"},
	{def::NOP, "NOP"}	
};

} // namespace instruction

inline uint8_t operator+(instruction::def i)	{
	return static_cast<uint8_t>(i);
}