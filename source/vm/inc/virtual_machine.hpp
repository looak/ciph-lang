#pragma once

#include <cstdint>
#include "execution_context.hpp"

class VirtualMachine
{
public:
    VirtualMachine(uint8_t* program, size_t size);
    ~VirtualMachine() = default;

    int32_t execute();

private:
    void run(ExecutionContext& context);
    
    uint8_t* m_program;
    size_t m_size;
};