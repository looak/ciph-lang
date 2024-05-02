#pragma once
#include <cstdint>
#include <algorithm>

namespace ciph {

template <size_t N>
class Memory {
public:
    Memory() {
        m_memory = new uint8_t[N];
        m_size = N;
        m_allocPointer = 0;
    };
    ~Memory() {
        delete[] m_memory;
    }

    uint16_t* allocate(uint16_t size) {
        size *= 2;
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
        if (size == 0)
            return m_allocPointer;
        
        std::copy(program, program + size, &m_memory[m_allocPointer]);
        uint16_t addrs = m_allocPointer;
        m_allocPointer += size;
        return addrs;
    }

    uint8_t* getMemory(uint16_t address) const {
        return &m_memory[address];
    }

    uint8_t* getMemory() const {
        return m_memory;
    }

private:
    uint8_t* m_memory;
    uint16_t m_size;
    uint16_t m_allocPointer;
};

} // namespace ciph