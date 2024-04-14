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
    uint8_t program[] = {
        +Instructions::PUSH, 0, 0, 0, 2,
        +Instructions::PUSH, 0, 0, 0, 2,
        +Instructions::MUL,
        +Instructions::PUSH, 0, 0, 0, 3,
        +Instructions::ADD,
        +Instructions::RET
    };
    
    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 7);
}

// ReturnStatment_Expression
TEST(VirtualMachineTest, OrderOfOperation_ParenthasesBeforeMul)
{
    // return (2 * (2 + 3)) should be 10    
    uint8_t program[] = {
        +Instructions::PUSH, 0, 0, 0, 2,
        +Instructions::PUSH, 0, 0, 0, 2,
        +Instructions::PUSH, 0, 0, 0, 3,
        +Instructions::ADD,
        +Instructions::MUL,
        +Instructions::RET
    };
    
    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 10);
}

TEST(VirtualMachineTest, ReturnStatment_Default)
{
    // return 
    uint8_t program[] = {         
        +Instructions::PUSH, 0, 0, 0, 0,
        +Instructions::RET }; // should return 0;
    
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

TEST(VirtualMachineTest, Return_Variable)
{
    // let x = 25 
    // return x
    uint8_t program[] = {
        +Instructions::PUSH, 0, 0, 0, 25,
        +Instructions::PEEK, 0, 0, 0, 0,
        +Instructions::RET
    };

    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 25);
}

TEST(VirtualMachineTest, Variables_Expression)
{
    // let x = 25 
    // let y = 10
    // return x + y
    uint8_t program[] = {
        +Instructions::PUSH, 0, 0, 0, 25,
        +Instructions::PUSH, 0, 0, 0, 10,
        +Instructions::PEEK, 0, 0, 0, 0,
        +Instructions::PEEK, 0, 0, 0, 1,
        +Instructions::ADD,
        +Instructions::RET
    };

    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 35);
}

TEST(VirtualMachine, Variable_BrokenExpression)
{
    uint8_t program[] = { +Instructions::PUSH, 0, 0, 0, 10,
                          +Instructions::PUSH, 0, 0, 0, 20,                                    
                          +Instructions::ADD,
                          +Instructions::PUSH, 0, 0, 0, 3,
                          +Instructions::PEEK, 0, 0, 0, 0,                                  
                          +Instructions::PEEK, 0, 0, 0, 1,
                          +Instructions::DIV,
                          +Instructions::RET};

    VirtualMachine vm(&program[0], sizeof(program));
    auto result = vm.execute();

    EXPECT_EQ(result, 10);
}