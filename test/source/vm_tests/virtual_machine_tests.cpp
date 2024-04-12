#include <gtest/gtest.h>
#include <instructions.hpp>
#include <virtual_machine.hpp>

TEST(VirtualMachineTest, AddingTwoIntegers)
{
    uint8_t program[] = {
        +Instructions::PUSH, 0, 0, 0, 25,
        +Instructions::PUSH, 0, 0, 0, 16,
        +Instructions::ADD
    };

    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 41);    
}