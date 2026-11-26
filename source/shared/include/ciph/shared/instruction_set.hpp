#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <string_view>
#include <utility>

#include <ciph/shared/shared_defines.hpp>

namespace ciph {
namespace instruction {

// Wraps a string literal so it can be a template argument.
template <size_t N>
struct FixedString {
	char data[N]{};
	constexpr FixedString(const char (&str)[N]) { std::copy_n(str, N, data); }
	constexpr std::string_view view() const { return { data, N - 1 }; }
};

// What an instruction *is*. Nothing here depends on the VM.
template <def Op, FixedString Mnemonic>
struct Instr {
	static constexpr def opcode = Op;
	static constexpr uint8_t value = std::to_underlying(Op);
	static constexpr std::string_view mnemonic = Mnemonic.view();
};

template <typename... Instrs>
struct InstructionSet {
	static constexpr std::array<uint8_t, sizeof...(Instrs)> opcodes{ Instrs::value... };

	static constexpr auto mnemonics = [] {
		std::array<std::string_view, 256> table{};
		table.fill("???");
		((table[Instrs::value] = Instrs::mnemonic), ...);
		return table;
	}();
};

consteval bool has_unique_opcodes(std::span<const uint8_t> opcodes) {
	std::array<bool, 256> seen{};
	for (uint8_t op : opcodes) {
		if (seen[op]) return false;
		seen[op] = true;
	}
	return true;
}

// Built from CIPH_INSTRUCTIONS in shared_defines.hpp; add instructions there.
#define CIPH_X(NAME, OP, MNEMONIC) Instr<def::NAME, MNEMONIC>,
#define CIPH_X_LAST(NAME, OP, MNEMONIC) Instr<def::NAME, MNEMONIC>
using AllInstructions = InstructionSet<CIPH_INSTRUCTIONS(CIPH_X, CIPH_X_LAST)>;
#undef CIPH_X_LAST
#undef CIPH_X

static_assert(has_unique_opcodes(AllInstructions::opcodes), "Duplicate instruction opcode");

} // namespace instruction
} // namespace ciph
