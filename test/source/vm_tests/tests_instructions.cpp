#include <gtest/gtest.h>
#include <instructions.hpp>

void helper_push(ExecutionContext& context, int& offset, int value)
{
    context.bytecode[offset] = +Instructions::PUSH;    
    context.bytecode[++offset] = static_cast<uint8_t>((value >> 24) & 0xFF);    
    context.bytecode[++offset] = static_cast<uint8_t>((value >> 16) & 0xFF);
    context.bytecode[++offset] = static_cast<uint8_t>((value >> 8) & 0xFF);
    context.bytecode[++offset] = static_cast<uint8_t>(value & 0xFF);
}

TEST(InstructionsTest, PushHandler)
{
    int offset = 0;
    ExecutionContext context;
    context.bytecode = new uint8_t[5];
    helper_push(context, offset, 10);    
    instruction::set[context.bytecode[context.pc]](context);
	EXPECT_EQ(context.stack.top(), 10);
}

int BinaryExpressionHandlerTest(int a, int b, Instructions instruction)
{
    int offset = 0;
	ExecutionContext context;
    context.bytecode = new uint8_t[11];
	helper_push(context, offset, a);
    helper_push(context, ++offset, b);
	context.bytecode[++offset] = +instruction;

    for (int i = 0; i < 3; i++)
    {
		instruction::set[context.bytecode[context.pc]](context);
        context.pc++;
	}
	
	return context.stack.top();
}

TEST(InstructionsTest, BinaryExpressionHandlersTests)
{
    std::vector<std::tuple<int, int, Instructions, int>> tests = {        
        std::make_tuple(7, 3, Instructions::ADD, 10),
        std::make_tuple(9, 5, Instructions::SUB, 4),
        std::make_tuple(3, 5, Instructions::MUL, 15),
        std::make_tuple(9, 3, Instructions::DIV, 3),
        // larger numbers
        std::make_tuple(2569,      33995, Instructions::MUL, 87333155),
        std::make_tuple(313195,    47806,  Instructions::SUB, 265389),
        std::make_tuple(-1,        1,      Instructions::ADD, 0),
        std::make_tuple(6551447,   3692,   Instructions::DIV, 177)
    };

    for (auto& test : tests)
    {        
        auto [a, b, instruction, expected] = test;
        int result = BinaryExpressionHandlerTest(a, b, instruction);
        EXPECT_EQ(result, expected);
    }
}

TEST(InstructionsTest, SubHandler)
{
    int offset = 0;
	ExecutionContext context;
    context.bytecode = new uint8_t[11];
	helper_push(context, offset, 10);
    helper_push(context, ++offset, 20);
	context.bytecode[++offset] = +Instructions::SUB;

    for (int i = 0; i < 3; i++)
    {
		instruction::set[context.bytecode[context.pc]](context);
        context.pc++;
	}
	
	EXPECT_EQ(context.stack.top(), -10);
}
