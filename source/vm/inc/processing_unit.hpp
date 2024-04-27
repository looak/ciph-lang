#pragma once
#include "memory.hpp"
#include "shared_defines.hpp"

struct Registers {
public:
    void set(uint16_t* mem) {
        imm = &mem[+registers::def::imm];
        r0 = &mem[+registers::def::r0];
        r1 = &mem[+registers::def::r1];
        r2 = &mem[+registers::def::r2];
        r3 = &mem[+registers::def::r3];
        r4 = &mem[+registers::def::r4];
        r5 = &mem[+registers::def::r5];
        r6 = &mem[+registers::def::r6];
        ret = &mem[+registers::def::ret];
        sp = &mem[+registers::def::sp];
        fp = &mem[+registers::def::fp];
        bp = &mem[+registers::def::bp];
        pc = &mem[+registers::def::pc];
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
