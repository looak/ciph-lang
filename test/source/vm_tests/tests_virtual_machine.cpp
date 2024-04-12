#include <gtest/gtest.h>
#include <instructions.hpp>
#include <virtual_machine.hpp>

TEST(VirtualMachineTest, AddingTwoIntegers)
{
    uint8_t program[] = {
        +Instructions::PUSH, 0, 0, 0, 25,
        +Instructions::PUSH, 0, 0, 0, 16,
        +Instructions::ADD,
        +Instructions::RET
    };

    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 41);    
}

TEST(VirtualMachineTest, OrderOfOperation_MulBeforeAdd)
{
    // return (2 * 2 + 3)
    uint8_t program[] = { 01,00,00,00,02,01,00,00,00,02,05,01,00,00,00,03,03, 8 }; 
    
    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 7);
}

TEST(VirtualMachineTest, OrderOfOperation_ParenthasesBeforeMul)
{
    // return (2 * (2 + 3)) should be 10
    uint8_t program[] = { 01,00,00,00,02,01,00,00,00,02,01,00,00,00,03,03,05, 8 }; 
    
    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 10);
}

TEST(VirtualMachineTest, ReturnStatment_Expression)
{
    // return (2 * (2 + 3)) should be 10
    uint8_t program[] = { 01,00,00,00,02,01,00,00,00,02,01,00,00,00,03,03,05, 8 }; 
    
    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 10);
}

TEST(VirtualMachineTest, ReturnStatment_Default)
{
    // return 
    uint8_t program[] = { 8 }; // should return 0;
    
    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 0);
}

TEST(VirtualMachineTest, Return_ReturnNumericLiteral)
{
    // return 25
    uint8_t program[] = {
        +Instructions::PUSH, 0, 0, 0, 25,
        +Instructions::RET
    };

    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 25);
}