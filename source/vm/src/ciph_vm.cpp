#include <fmt/core.h>
#include <fstream>
#include "virtual_machine.hpp"

int main(int argc, char *argv[]) {

    // check that we have the right number of arguments
    if (argc != 2) {
        fmt::print(stderr, "Usage: {} <filename>\n", argv[0]);
        return 1;
    }

    // open compiled file
    std::fstream file(argv[1]);
    if (!file.is_open()) {
        fmt::print(stderr, "Error: could not open file '{}'\n", argv[1]);
        return 1;
    }
    uint32_t size;
    file.read(reinterpret_cast<char *>(&size), sizeof(uint32_t));

    // read bytecode
    uint8_t *bytecode = new uint8_t[size];
    file.read(reinterpret_cast<char*>(bytecode), size);

    VirtualMachine vm(bytecode, size);
    int32_t result = vm.execute();

    fmt::print("Program exited with code {}\n", result);

    return 0;
    
    
}