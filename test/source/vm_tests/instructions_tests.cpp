#include <gtest/gtest.h>
#include <instructions.hpp>

void helper_push(ExecutionContext& context, int value)
{
    context.bytecode.push_back(+Instructions::PUSH);
    context.bytecode.push_back(value);    
}

TEST(InstructionsTest, PushHandler)
{
    ExecutionContext context;
    helper_push(context, 10);    
    instruction::set[context.bytecode[context.pc]](context);
	EXPECT_EQ(context.stack.top(), 10);
}

TEST(InstructionsTest, AddHandler)
{
	ExecutionContext context;
	helper_push(context, 10);    
    helper_push(context, 20);
	context.bytecode.push_back(+Instructions::ADD);

    for (int i = 0; i < 3; i++)
    {
		instruction::set[context.bytecode[context.pc]](context);
        context.pc++;
	}

	//instruction::set[+Instructions::ADD](context);
	EXPECT_EQ(context.stack.top(), 30);
}
