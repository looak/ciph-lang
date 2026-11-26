#pragma once

#include <array>

#include <ciph/shared/instruction_set.hpp>
#include <ciph/shared/shared_defines.hpp>
#include <ciph/vm/execution_context.hpp>

namespace ciph {
namespace instruction {

int16_t stack_read_at_offset(uint8_t* bytecode, uint16_t& sp);
int16_t read_word(uint8_t* bytecode, uint16_t& pc);
void write_int16(uint8_t* bytecode, uint16_t& pc, int16_t value);
int16_t peek_helper(uint8_t* bytecode, uint16_t sp);
int16_t pop_helper(ExecutionContext& context);
void push_helper(ExecutionContext& context, int16_t value);
void push_helper_reg(ExecutionContext& context, registers::def reg);

void jmp_helper(ExecutionContext& context, int16_t address);

using Handler = void (*)(ExecutionContext& context);

// One handler per opcode. Each is an ordinary function in instructions.cpp.
// Every instruction in CIPH_INSTRUCTIONS gets a declaration here and needs a
// definition there, or linking fails.
template <def Op>
void exec(ExecutionContext& context);

// Declares exec<def::NAME> for every implemented instruction. Only the
// declarations come from the macro; the definitions are written out in full.
#define CIPH_DECLARE_EXEC(NAME, OP, MNEMONIC) template <> void exec<def::NAME>(ExecutionContext& context);
CIPH_INSTRUCTIONS(CIPH_DECLARE_EXEC, CIPH_DECLARE_EXEC)
#undef CIPH_DECLARE_EXEC

// Runs for any opcode that isn't in AllInstructions: stops the VM.
inline void invalid_handler(ExecutionContext& context) {
	context.faulted = true;
}

template <typename Set>
struct Dispatch;

template <typename... Instrs>
struct Dispatch<InstructionSet<Instrs...>> {
	static constexpr std::array<Handler, 256> table = [] {
		std::array<Handler, 256> t{};
		t.fill(&invalid_handler);
		((t[Instrs::value] = &exec<Instrs::opcode>), ...);
		return t;
	}();
};

// Indexed by opcode byte: handlers[+def::ADD](context).
inline constexpr const std::array<Handler, 256>& handlers = Dispatch<AllInstructions>::table;

} // namespace instruction
} // namespace ciph
