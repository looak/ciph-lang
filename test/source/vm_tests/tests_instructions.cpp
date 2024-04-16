#include <gtest/gtest.h>
#include <instructions.hpp>
#include <processing_unit.hpp>
#include <memory.hpp>


int16_t peek_word(uint8_t* bytecode, int16_t offset)
{
    int16_t value = 0;
    value = (value << 8) | bytecode[++offset];
    value = (value << 8) | bytecode[++offset];
    return value;
}

class InstructionsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        registries = mem.allocate(static_cast<uint16_t>(Register::reg_cnt));
        uint16_t addrs = mem.load(nullptr, 0);
        registries[+Register::pc] = addrs;
        registries[+Register::bp] = addrs;

        // setting up an arbitrary stack location,
        // 64 bytes from the start of the memory
        uint16_t stackAddrs = static_cast<uint16_t>(addrs + 64);
        registries[+Register::sp] = stackAddrs;
        registries[+Register::fp] = stackAddrs;
    }
    
    Memory<0x100> mem;
    uint16_t* registries = nullptr;
};


TEST_F(InstructionsTest, PushHandler)
{
    uint8_t program[] = {
        +instruction::def::PSH_LIT, 0x0d, 0xf0 // push 0xf00d 
    };

    mem.load(program, sizeof(program));

    ExecutionContext context(registries, mem.getMemory());
    uint16_t pc = registries[+Register::pc];
    auto instruction = static_cast<instruction::def>(context.bytecode[pc]);

    EXPECT_EQ(instruction, instruction::def::PSH_LIT);

    instruction::handlers[instruction](context);
    int16_t value = peek_word(context.bytecode, registries[+Register::sp] - 2);
	EXPECT_EQ(value, 0xf00d);
}

// int BinaryExpressionHandlerTest(int a, int b, Instructions instruction)
// {
//     int offset = 0;
// 	ExecutionContext context;
//     context.bytecode = new uint8_t[11];
// 	helper_push(context, offset, a);
//     helper_push(context, ++offset, b);
// 	context.bytecode[++offset] = +instruction;

//     for (int i = 0; i < 3; i++)
//     {
//         auto instr = static_cast<Instructions>(context.bytecode[context.pc]);
// 		instruction::handlers[instr](context);
//         context.pc++;
// 	}
	
// 	return context.stack.back();
// }

// TEST(InstructionsTest, BinaryExpressionHandlersTests)
// {
//     std::vector<std::tuple<int, int, Instructions, int>> tests = {        
//         std::make_tuple(7, 3, Instructions::ADD, 10),
//         std::make_tuple(9, 5, Instructions::SUB, 4),
//         std::make_tuple(3, 5, Instructions::MUL, 15),
//         std::make_tuple(9, 3, Instructions::DIV, 3),
//         // larger numbers
//         std::make_tuple(2569,      33995, Instructions::MUL, 87333155),
//         std::make_tuple(313195,    47806,  Instructions::SUB, 265389),
//         std::make_tuple(-1,        1,      Instructions::ADD, 0),
//         std::make_tuple(6551447,   3692,   Instructions::DIV, 1774)
//     };

//     for (auto& test : tests)
//     {        
//         auto [a, b, instruction, expected] = test;
//         int result = BinaryExpressionHandlerTest(a, b, instruction);
//         EXPECT_EQ(result, expected);
//     }
// }

// TEST(InstructionsTest, SubHandler)
// {
//     int offset = 0;
// 	ExecutionContext context;
//     context.bytecode = new uint8_t[11];
// 	helper_push(context, offset, 10);
//     helper_push(context, ++offset, 20);
// 	context.bytecode[++offset] = +Instructions::SUB;

//     for (int i = 0; i < 3; i++)
//     {
//         auto instruction = static_cast<Instructions>(context.bytecode[context.pc]);
// 		instruction::handlers[instruction](context);
//         context.pc++;
// 	}
	
// 	EXPECT_EQ(context.stack.back(), -10);
// }

// TEST(InstructionsTest, PeekHandler)
// {
//     int offset = 0;
//     ExecutionContext context;
//     context.bytecode = new uint8_t[4];
//     write_value(context, offset, 1);
//     context.stack.push_back(10);
//     context.stack.push_back(20);
//     context.stack.push_back(30);

//     instruction::peek_handler(context);
//     EXPECT_EQ(context.stack.back(), 20);
//     EXPECT_EQ(context.stack.size(), 4);
// }