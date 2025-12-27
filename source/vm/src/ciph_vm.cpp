#include <fmt/core.h>
#include <windows.h>

#include <fstream>
#include <iostream>

#include "code_generator.hpp"
#include "disassembler.hpp"
#include "parser.hpp"
#include "processing_unit.hpp"
#include "shared_defines.hpp"

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void PrintProgramAndCurrentCount(ProcessingUnit& pu, Disassembler& disassembler, int x, int y) {
    const auto& disassemblyMap = disassembler.disassembledInstructions();

    const uint16_t& programCount = pu.registries()[+registers::def::pc];
    const uint16_t& baseProgram = pu.registries()[+registers::def::bp];

    uint16_t currProgram = programCount - baseProgram;
    int yOffset = y;
    for (const auto& [key, instruction] : disassemblyMap) {
        if (key == currProgram) {
            gotoxy(x - 3, yOffset);
            fmt::print("->");
        }
        gotoxy(x, yOffset++);
        fmt::print("0x{:04X}  {}", key, instruction);
    }
}

void PrintStack(ProcessingUnit& pu, int x, int y) {
    uint16_t stackSize = pu.registries()[+registers::def::sp] - pu.registries()[+registers::def::fp];
    stackSize /= 2;

    gotoxy(x, y);
    fmt::println("Stack: {}", stackSize);
    uint16_t cnt = 0;

    uint16_t stackTop = pu.registries()[+registers::def::sp];
    uint16_t stackEnd = pu.registries()[+registers::def::fp];

    while (stackTop > stackEnd) {
        int16_t value = 0;

        value = (value << 8) | pu.memory()[--stackTop];
        value = (value << 8) | pu.memory()[--stackTop];
        gotoxy(x, ++y);
        fmt::println("0x{} [offset: {}] {:04X}", stackTop, stackSize - ++cnt, value);
    }
}

void PrintRegisters(ProcessingUnit& pu, int x, int y) {
    auto regs = pu.registries();
    gotoxy(x, y);
    fmt::println("Registers:");
    gotoxy(x, ++y);
    gotoxy(x, ++y);
    fmt::println("imm: 0x{:04X}", regs[+registers::def::imm]);
    gotoxy(x, ++y);
    fmt::println("r0:  0x{:04X}", regs[+registers::def::r0]);
    gotoxy(x, ++y);
    fmt::println("r1:  0x{:04X}", regs[+registers::def::r1]);
    gotoxy(x, ++y);
    fmt::println("r2:  0x{:04X}", regs[+registers::def::r2]);
    gotoxy(x, ++y);
    fmt::println("r3:  0x{:04X}", regs[+registers::def::r3]);
    gotoxy(x, ++y);
    fmt::println("r4:  0x{:04X}", regs[+registers::def::r4]);
    gotoxy(x, ++y);
    fmt::println("r5:  0x{:04X}", regs[+registers::def::r5]);
    gotoxy(x, ++y);
    fmt::println("r6:  0x{:04X}", regs[+registers::def::r6]);
    gotoxy(x, ++y);
    fmt::println("ret: 0x{:04X}", regs[+registers::def::ret]);
    gotoxy(x, ++y);
    fmt::println("sp:  0x{:04X}", regs[+registers::def::sp]);
    gotoxy(x, ++y);
    fmt::println("fp:  0x{:04X}", regs[+registers::def::fp]);
    gotoxy(x, ++y);
    fmt::println("bp:  0x{:04X}", regs[+registers::def::bp]);
    gotoxy(x, ++y);
    fmt::println("pc:  0x{:04X}", regs[+registers::def::pc]);
    gotoxy(x, ++y);
}

int main(int argc, char* argv[]) {

    bool running = true;
    std::string input("return 5 + 5");

    // std::string input = "";
    //  std::getline(std::cin, input); // wait for user to press enter
    //  if (input == "exit")
    //      return 0;

    Parser parser(input);
    auto abstract_program = parser.parse();
    CodeGenerator code_generator(reinterpret_cast<ASTProgramNode*>(abstract_program));
    code_generator.generateCode();
    auto [program, psize] = code_generator.readRawBytecode();


    ProcessingUnit pu;
    pu.load_program(&program[0], psize);
    Disassembler disassembler(&program[0], psize);
    disassembler.disassemble();

    running = true;
    while (running) {
        system("cls");

        PrintProgramAndCurrentCount(pu, disassembler, 5, 2);
        PrintStack(pu, 32, 2);
        PrintRegisters(pu, 64, 2);
        running = pu.step();

        gotoxy(0, 24);
        std::getline(std::cin, input);
        if (input == "exit")
            running = false;
    }

    system("cls");

    PrintProgramAndCurrentCount(pu, disassembler, 5, 2);
    PrintStack(pu, 32, 2);
    PrintRegisters(pu, 64, 2);

    gotoxy(0, 24);
    fmt::println("Program returned: {}", pu.context().return_value);
    std::getline(std::cin, input); // wait for user to press enter
    return 0;
}