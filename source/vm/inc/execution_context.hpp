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
	ExecutionContext(size_t size, uint16_t& pc, uint16_t& sp)
		: pc(pc), sp(sp)
	{}

	uint16_t& pc;
	uint16_t& sp;
	
	int32_t return_value = 0;
};