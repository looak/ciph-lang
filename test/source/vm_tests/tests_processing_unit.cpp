#include <gtest/gtest.h>

#include <shared_defines.hpp>
#include "processing_unit.hpp"

uint8_t operator+(Instruction instr) {
    return static_cast<uint8_t>(instr);
}

TEST(ProcessingUnitTest, MustStartSomeWhere)
{
    ProcessingUnit unit;
    //unit.reset();
    unit.load_program(nullptr, 0);
    //unit.execute();
    ASSERT_TRUE(true);
}

TEST(ProcessingUnitTest, LoadSimpleProgram)
{
    ProcessingUnit unit;
    //unit.reset();
    uint8_t program[] = { 
        +Instruction::PSH, 0, 25,
        +Instruction::PSH, 0, 16,
        +Instruction::ADD,
        +Instruction::RET};
    unit.load_program(program, sizeof(program));

    EXPECT_EQ(10, unit.registries()[+Register::pc]);
    unit.execute();
    ASSERT_TRUE(true);
}
