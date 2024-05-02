#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#define i16(x) static_cast<int16_t>(x)
#define u16(x) static_cast<uint16_t>(x)
#define u8(x) uint8_t(x)

namespace ciph {

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
	INC		=		0x0E, 	// Increases the value at given register by one, if reg:sp is given, value on stack is incremented at given offset.
 	DEC 	=		0x0F, 	// Same as increment, just decreases the value by one.
 
	
	// Stack instructions
	PSH	 	=		0x10, 	// Pushes data from imm register onto stack.
	PSH_REG	=		0x20, 	// Pushes data from reg onto stack.
	PSH_LIT	=		0x11, 	// Pushes literal onto stack.
	PSH_MEM	=		0x21, 	// Takes value at memory adress and pushes it onto stack.
	POP	 	=		0x30, 	// Pops top of stack to given imm register
	POP_REG	=		0x40, 	// Pops top of stack to given register.
	PEK_REG	=		0x50, 	// Copies top value of stack into given register
	PEK_OFF	=		0x51, 	// Copies value of stack at offset into given register
	
	// Control flow instructions
	JMP	 	=		0xC0, 	// Unconditionally jump to address.
	JNZ	 	=		0xC1, 	// Jump to address if imm is not zero.	
	JEQ 	=		0xC2,  	// Jump to address if imm is zero.
	JGT 	=		0xC3,  	// Jump to address if imm is positive.
	JLT 	=		0xC4,  	// Jump to address if imm is negative.
	CMP 	= 		0xCC, 	// Subtracts rX from rY and puts result in imm, if reg:sp is passed as rX we pop the compared elements from the stack
	RET	 	=		0xCF, 	// Returns value in imm and terminates the program.
 
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
	{def::INC, "INC"},
	{def::DEC, "DEC"},
	{def::PSH, "PSH"},
	{def::PSH_REG, "PSH"},
	{def::PSH_LIT, "PSH"},
	{def::PSH_MEM, "PSH"},
	{def::POP, "POP"},
	{def::POP_REG, "POP"},
	{def::PEK_REG, "PEK"},
	{def::PEK_OFF, "PEK"},
	{def::JMP, "JMP"},
	{def::JNZ, "JNZ"},
	{def::JLT, "JLT"},
	{def::CMP, "CMP"},
	{def::RET, "RET"},
	{def::NOP, "NOP"}
};

} // namespace instruction

namespace registers {
enum class def : uint8_t {
    imm = 0x00,
    r0 = 0x01,
    r1,
    r2,
    r3,
    r4,
    r5,
    r6 = 0x07,
    ret = 0x08,
    sp = 0x0A,
    fp = 0x0B,
    bp = 0x0E,
    pc = 0x0F,
    reg_cnt = 0x10
};

const std::unordered_map<def, std::string> name = {
	{ def::imm, "imm" },
	{ def::r0, "r0" },
	{ def::r1, "r1" },
	{ def::r2, "r2" },
	{ def::r3, "r3" },
	{ def::r4, "r4" },
	{ def::r5, "r5" },
	{ def::r6, "r6" },
	{ def::ret, "ret" },
	{ def::sp, "sp" },
	{ def::fp, "fp" },
	{ def::bp, "bp" },
	{ def::pc, "pc" }
};

} // namespace register

inline uint8_t operator+(registers::def reg) {
    return static_cast<uint8_t>(reg);
}

inline uint8_t operator+(instruction::def i)	{
	return static_cast<uint8_t>(i);
}

} // namespace ciph