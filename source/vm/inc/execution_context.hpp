#pragma once

#include <cstdint>

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

	uint16_t* registry;	
	uint8_t* bytecode = nullptr;
	int16_t return_value = 0;
};