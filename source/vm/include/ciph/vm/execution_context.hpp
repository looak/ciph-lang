#pragma once

#include <cstdint>
#include <utility>

#include <ciph/shared/shared_defines.hpp>

namespace ciph {

struct ExecutionContext 
{
public:
	ExecutionContext(uint16_t* reg, uint8_t* _bytecode)
		: registry(reg)
		, bytecode(_bytecode)
	{}

	ExecutionContext()
		: registry(nullptr)
		, bytecode(nullptr)
	{}

	// The register `r` holds. Prefer this to indexing `registry` directly.
	uint16_t& reg(registers::def r) const { return registry[std::to_underlying(r)]; }

	uint16_t* registry;
	uint8_t* bytecode = nullptr;
	int16_t return_value = 0;
};

} // namespace ciph