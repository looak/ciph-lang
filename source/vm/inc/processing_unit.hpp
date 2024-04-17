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
    r6 = 0x07,
    ret = 0x08,
    sp = 0x0A,
    fp = 0x0B,
    bp = 0x0E,
    pc = 0x0F,
    reg_cnt = 0x10
};

inline uint8_t operator+(Register reg) {
    return static_cast<uint8_t>(reg);
}

struct Registers {
public:
    void set(uint16_t* mem) {
        imm = &mem[+Register::imm];
        r0 = &mem[+Register::r0];
        r1 = &mem[+Register::r1];
        r2 = &mem[+Register::r2];
        r3 = &mem[+Register::r3];
        r4 = &mem[+Register::r4];
        r5 = &mem[+Register::r5];
        r6 = &mem[+Register::r6];
        ret = &mem[+Register::ret];
        sp = &mem[+Register::sp];
        fp = &mem[+Register::fp];
        bp = &mem[+Register::bp];
        pc = &mem[+Register::pc];
    }
    uint16_t* imm;
    uint16_t* r0;
    uint16_t* r1;
    uint16_t* r2;
    uint16_t* r3;
    uint16_t* r4;
    uint16_t* r5;
    uint16_t* r6;
    uint16_t* ret;
    uint16_t* sp;
    uint16_t* fp;
    uint16_t* bp;
    uint16_t* pc;
};

class ProcessingUnit {
public:
    ProcessingUnit();
    
    void load_program(uint8_t* program, uint16_t size);

    int16_t execute();

    uint16_t* registries() const {
        return m_reg_memory;
    }

private:
    
    Registers m_registers;
    uint16_t* m_reg_memory;
    
    //uint16_t* m_registries;

    Memory<0x1000> m_memory; // 4KB
    uint8_t* ptr = nullptr;
};
