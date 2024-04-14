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
	ExecutionContext() = default;
    ~ExecutionContext() = default;

	uint32_t pc = 0;
	uint32_t sp = 0;

	size_t program_size = 0;
	int32_t return_value = 0;

	uint8_t* bytecode;
	std::vector<int32_t> stack;
};