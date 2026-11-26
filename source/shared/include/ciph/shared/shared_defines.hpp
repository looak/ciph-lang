#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#define i16(x) static_cast<int16_t>(x)
#define u16(x) static_cast<uint16_t>(x)
#define u8(x) uint8_t(x)

namespace ciph {

namespace instruction {

// The instruction set, written once. Each entry is X(name, opcode, mnemonic).
// Every user defines X to say what an entry becomes (an enum value, an Instr<>,
// an exec<> declaration) and expands the list with it.
//
// CIPH_INSTRUCTIONS holds the instructions the VM implements. Its final entry
// uses LAST instead of X, so a user that separates entries with commas can leave
// the comma off the last one. Add new instructions above HALT.
//
// Comments in the lists must be /* */: a // comment would swallow the
// line-continuation backslash.
#define CIPH_INSTRUCTIONS(X, LAST) \
	/* Data instructions */ \
	X(MOV,     0x02, "MOV")  /* Move data from register rY to register rX, if rY is unspecified moves rX into imm register. */ \
	\
	/* Arithmetic instructions */ \
	X(NEG,     0xF0, "NEG")  /* Negates the value at the top of the stack. */ \
	X(ADD,     0x06, "ADD")  /* Pops two values from the stack and adds them together. Result is pushed onto stack. */ \
	X(SUB,     0x08, "SUB")  /* Subtracts second value on stack with first and pushes difference onto stack. */ \
	X(MUL,     0x0A, "MUL")  /* Pops two values from the stack and multiplies them together. */ \
	X(DIV,     0x0C, "DIV")  /* Pops divisor of the stack, then pops dividen of the stack. Puts quotient of the divison onto stack. Remainder is lost. */ \
	X(INC,     0x0E, "INC")  /* Increases the value at given register by one, if reg:sp is given, value on stack is incremented at given offset. */ \
	X(DEC,     0x0F, "DEC")  /* Same as increment, just decreases the value by one. */ \
	\
	/* Stack instructions */ \
	X(PSH,     0x10, "PSH")  /* Pushes data from imm register onto stack. */ \
	X(PSH_REG, 0x20, "PSH")  /* Pushes data from reg onto stack. */ \
	X(PSH_LIT, 0x11, "PSH")  /* Pushes literal onto stack. */ \
	X(POP_REG, 0x40, "POP")  /* Pops top of stack to given register. */ \
	X(PEK_REG, 0x50, "PEK")  /* Copies top value of stack into given register. */ \
	X(PEK_OFF, 0x51, "PEK")  /* Copies value of stack at offset into given register. */ \
	\
	/* Control flow instructions */ \
	X(JMP,     0xC0, "JMP")  /* Unconditionally jump to address. */ \
	X(JNZ,     0xC1, "JNZ")  /* Jump back by the given distance if imm is not zero. */ \
	X(JEQ,     0xC2, "JEQ")  /* Jump back by the given distance if imm is zero. */ \
	X(JGT,     0xC3, "JGT")  /* Jump back by the given distance if imm is positive. */ \
	X(JLT,     0xC4, "JLT")  /* Jump back by the given distance if imm is negative. */ \
	X(CALL,    0xCA, "CALL") /* Pushes the return address and fp, points fp at the new frame and jumps to address. */ \
	X(CMP,     0xCC, "CMP")  /* Subtracts rX from rY and puts result in imm, if reg:sp is passed as rX we pop the compared elements from the stack. */ \
	X(RET,     0xCF, "RET")  /* Returns value in imm. */ \
	LAST(HALT, 0xFE, "HALT") /* Terminates the program. */

// Opcodes that are part of the encoding but have no VM handler yet. They get
// an enum value and nothing else. To implement one, move its line into
// CIPH_INSTRUCTIONS and write its exec<> in instructions.cpp.
#define CIPH_RESERVED_INSTRUCTIONS(X) \
	/* Data instructions */ \
	X(PUT,     0x01, "PUT")  /* Takes the memory at the location and puts it in imm register. */ \
	X(PUT_REG, 0x04, "PUT")  /* Takes value of imm and puts it in given register. */ \
	X(PUT_LIT, 0x05, "PUT")  /* Move literal into imm register. */ \
	X(MOV_MEM, 0x03, "MOV")  /* Move data from imm register into given Memory. */ \
	\
	/* Arithmetic instructions */ \
	X(ADD_REG, 0x07, "ADD")  /* Takes values at registers, adds them together and puts result into rX. If rY is unspecified adds imm to rX. */ \
	X(SUB_REG, 0x09, "SUB")  /* Subtracts rX with rY and puts difference into rX. If rY is unspecified uses imm. */ \
	X(MUL_REG, 0x0B, "MUL")  /* Takes values at registers, multiplies them together and puts product into rX. If rY is unspecified uses imm as multiplier, still stores product in rX. */ \
	X(DIV_REG, 0x0D, "DIV")  /* Uses register rX as dividen and rY as divisor. Quotient is put into rX. Remainder is lost. If rY is unspecified uses imm as divisor. */ \
	\
	/* Stack instructions */ \
	X(PSH_MEM, 0x21, "PSH")  /* Takes value at memory adress and pushes it onto stack. */ \
	X(POP,     0x30, "POP")  /* Pops top of stack to given imm register. */ \
	\
	/* Other instructions */ \
	X(NOP,     0x00, "NOP")  /* No operation instruction, program counter should just tick pass this. */

enum class def : uint8_t {
#define CIPH_X(NAME, OP, MNEMONIC) NAME = OP,
	CIPH_INSTRUCTIONS(CIPH_X, CIPH_X)
	CIPH_RESERVED_INSTRUCTIONS(CIPH_X)
#undef CIPH_X
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