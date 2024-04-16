#pragma once
#include <cstdint>
#include <algorithm>

class Memory {
public:
    Memory() {
        m_memory = new uint8_t[0x1000] {0xEE}; // 4KB
        m_size = 0x1000;
        m_allocPointer = 0;
    };
    ~Memory() {
        delete[] m_memory;
    }

    uint16_t* allocate(uint16_t size) {
        uint16_t* ptr = reinterpret_cast<uint16_t*>(&m_memory[m_allocPointer]);
        std::fill_n(&m_memory[m_allocPointer], size, 0x00); // fill with 0x0
        m_allocPointer += size;        
        return ptr;
    }

    uint16_t load(uint8_t* program, uint16_t size) {
        if (m_allocPointer + size > m_size) {
            return 0x0;
            // out of memory
        }

        std::copy(program, program + size, &m_memory[m_allocPointer]);
        uint16_t addrs = m_memory[m_allocPointer];
        m_allocPointer += size;
        return addrs;
    }

    void writeByte(uint16_t address, uint8_t value);
    void writeWord(uint16_t address, uint16_t value);
    void writeDWord(uint16_t address, uint32_t value);
    void writeQWord(uint16_t address, uint64_t value);

    uint8_t readByte(uint16_t address);
    uint16_t readWord(uint16_t address);
    uint32_t readDWord(uint16_t address);
    uint64_t readQWord(uint16_t address);

private:
    uint8_t* m_memory;
    uint16_t m_size;
    uint16_t m_allocPointer;
};