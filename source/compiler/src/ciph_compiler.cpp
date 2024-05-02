#include "lexar.hpp"
#include "shared_defines.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include "code_generator.hpp"
#include "parser.hpp"

int
main(int argc, char* argv[])
{
    //if (argc < 3)
    //{
    //    fmt::print("Usage: <input> <output>\n");
    //    return 0;
    //}

    //std::string input(argv[1]);
    //std::ifstream code(input);
    //if (!code.is_open())
    //{
    //    fmt::print("Could not open file: {}\n", input);
    //    return 0;
    //}
    //
    //std::stringstream buffer;
    //buffer << code.rdbuf();    

    //std::string output_filename(argv[2]);    
    //std::ofstream file(output_filename);
    //// write to file

    //// 1. Parser
    //Parser parser(buffer.str());
    //CodeGenerator generator(parser.parseProgram());

    //generator.generateCode();
    //auto [bytecode, size] = generator.readRawBytecode();
    //std::streamsize streamSize = static_cast<std::streamsize>(size);
    //uint32_t size32 = static_cast<uint32_t>(size);
    //file.write(reinterpret_cast<const char*>(&size32), sizeof(uint32_t));
    //file.write(reinterpret_cast<const char*>(bytecode), streamSize);
    //
    //fmt::print(file, "{}", generator.disassemble());

    //delete[] bytecode;

    return 0;
}