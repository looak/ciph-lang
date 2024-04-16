#pragma once

#include <cstdint>
#include <vector>
#include <stack>


struct StackValue 
{
	union {
        int32_t i32;
		float f32;
	};
};

struct ExecutionContext 
{
public:
	ExecutionContext(uint16_t* reg, uint8_t* _bytecode)
		: registry(reg)
		, bytecode(_bytecode)
	{}

	uint16_t* registry;	
	uint8_t* bytecode = nullptr;
	int16_t return_value = 0;
};