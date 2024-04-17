#include <gtest/gtest.h>

#include <shared_defines.hpp>
#include "processing_unit.hpp"

// TEST(ProcessingUnitTest, MustStartSomeWhere)
// {
//     ProcessingUnit unit;
//     //unit.reset();
//     unit.load_program(nullptr, 0);
//     //unit.execute();
//     ASSERT_TRUE(true);
// }

TEST(ProcessingUnitTest, LoadSimpleProgram)
{
    ProcessingUnit unit;
    //unit.reset();
    uint8_t program[] = { 
        +instruction::def::PSH_LIT, 0, 25,
        +instruction::def::PSH_LIT, 0, 16,
        +instruction::def::ADD,
        +instruction::def::RET};
    unit.load_program(program, sizeof(program));

    EXPECT_EQ(32, unit.registries()[+Register::pc]);
    int16_t result = unit.execute();
    EXPECT_EQ(41, result);
}
