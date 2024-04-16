#pragma once
#include "memory.hpp"

enum class Register : uint8_t {
    imm = 0x00,
    r0 = 0x01,
    r1,
    r2,
    r3,
    r4,
    r5,
    r6,
    r7 = 0x08,
    sp = 0x0A,
    fp = 0x0B,
    bp = 0x0E,
    pc = 0x0F,
    reg_cnt = 0x10
};

inline uint8_t operator+(Register reg) {
    return static_cast<uint8_t>(reg);
}

// class ProgramContext {
// public:
//     ProgramContext(uint8_t* program, size_t size);

   


// };

class ProcessingUnit {
public:
    ProcessingUnit();
    
    void load_program(uint8_t* program, uint16_t size);

    void execute();

private:
    uint16_t* m_registries;
    Memory m_memory;
    uint8_t* ptr = nullptr;
};
