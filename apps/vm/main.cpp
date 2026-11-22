#include <fmt/core.h>

#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <variant>

#include <ciph/compiler/code_generator.hpp>
#include <ciph/shared/disassembler.hpp>
//#include <ciph/compiler/error_reporter.hpp>
#include <ciph/compiler/parser.hpp>
#include <ciph/vm/processing_unit.hpp>
#include <ciph/shared/shared_defines.hpp>

using namespace ciph;

void
gotoxy(int x, int y) {
#ifdef _WIN32
    COORD coord;
    coord.X = static_cast<SHORT>(x);
    coord.Y = static_cast<SHORT>(y);
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
    // ANSI escape: rows/columns are 1-based
    fmt::print("[{};{}H", y + 1, x + 1);
#endif
}

void
PrintProgramAndCurrentCount(ProcessingUnit& pu, Disassembler& disassembler, int x, int y) {
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

void
PrintStack(ProcessingUnit& pu, int x, int y) {
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

void
PrintRegisters(ProcessingUnit& pu, int x, int y) {
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

// Reads a whole file as bytes. Returns false if it could not be read.
bool
read_bytecode(const char* path, std::vector<uint8_t>& bytecode) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    bytecode.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return true;
}

// Steps through a loaded program, redrawing the disassembly, stack and
// registers after every instruction. Enter advances; "exit" stops.
int
run_stepper(uint8_t* program, uint16_t psize) {
    std::string input;

    ProcessingUnit pu;
    pu.load_program(program, psize);
    Disassembler disassembler(program, psize);
    disassembler.disassemble();

    bool running = true;
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

// ciph-vm <bytecode>   steps through a compiled program
// ciph-vm              reads ciph source typed into the console, compiles it,
//                      then steps through it
int
main(int argc, char* argv[]) {
    if (argc == 2) {
        std::vector<uint8_t> bytecode;
        if (!read_bytecode(argv[1], bytecode)) {
            fmt::println(stderr, "ciph-vm: could not read '{}'", argv[1]);
            return 2;
        }
        if (bytecode.empty()) {
            fmt::println(stderr, "ciph-vm: '{}' is empty", argv[1]);
            return 2;
        }
        return run_stepper(bytecode.data(), static_cast<uint16_t>(bytecode.size()));
    }
    if (argc > 2) {
        fmt::println(stderr, "usage: ciph-vm [<bytecode>]");
        return 2;
    }

    std::string input("return 5 + 5");

    std::vector<std::string> input_lines;
    std::string line;

    std::cout << "Enter your text (enter an empty line to finish):\n";

    while (std::getline(std::cin, line)) {
        if (line.empty()) { 
            break; // Stop when an empty line is entered
        }
        input_lines.push_back(line);
    }

    input.clear();
    for (const auto& toPrint : input_lines) {
        input += toPrint;
    }

    ASTBaseNode* abstract_program = nullptr;
    Parser parser(input);
    auto parse_result = parser.parse();
    if (parse_result) {
        abstract_program = *parse_result;
    }
    else {
        // ErrorReport report = {.code = parse_error.code,
        //                       .line = parse_error.position.line,
        //                       .column = parse_error.position.column,
        //                       .message = parse_error.additionalInfo};

        // log::error(report);
        return 1;
    }


    CodeGenerator code_generator(reinterpret_cast<ASTProgramNode*>(abstract_program));
    code_generator.generateCode();
    auto [program, psize] = code_generator.readRawBytecode();


    return run_stepper(&program[0], static_cast<uint16_t>(psize));
}