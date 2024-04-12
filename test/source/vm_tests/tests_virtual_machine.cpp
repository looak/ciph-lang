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

TEST(VirtualMachineTest, OrderOfOperation_MulBeforeAdd)
{
    uint8_t program[] = { 01,00,00,00,02,01,00,00,00,02,05,01,00,00,00,03,03 }; // (2 * 2 + 3) should be 7    
    
    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 7);
}

TEST(VirtualMachineTest, OrderOfOperation_ParenthasesBeforeMul)
{
    uint8_t program[] = { 01,00,00,00,02,01,00,00,00,02,01,00,00,00,03,03,05 }; // (2 * (2 + 3)) should be 10
    
    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 10);
}