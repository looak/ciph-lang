#include <gtest/gtest.h>

#include <shared_defines.hpp>
#include "processing_unit.hpp"

uint16_t testProcessingUnit(uint8_t* program, uint16_t size)
{
    ProcessingUnit unit;
    unit.load_program(program, size);
    return unit.execute();
}

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


TEST(ProcessingUnitTest, Variables_Expression)
{
    ProcessingUnit unit;

    // let x = 25 
    // let y = 10
    // return x + y
    uint8_t program[] = {
        +instruction::def::PSH_LIT, 0, 25,
        +instruction::def::PSH_LIT, 0, 10,
        +instruction::def::PEK_OFF, 0, 0,
        +instruction::def::PEK_OFF, 0, 1,
        +instruction::def::ADD,
        +instruction::def::RET
    };

    unit.load_program(program, sizeof(program));
    int16_t result = unit.execute();
    
    EXPECT_EQ(35, result);
}

TEST(ProcessingUnitTest, AddingTwoIntegers)
{
    uint8_t program[] = {
        +instruction::def::PSH_LIT, 0, 25,
        +instruction::def::PSH_LIT, 0, 16,
        +instruction::def::ADD,
        +instruction::def::RET
    };

    uint16_t result = testProcessingUnit(program, sizeof(program));
    EXPECT_EQ(result, 41);
}

TEST(ProcessingUnitTest, OrderOfOperation_MulBeforeAdd)
{
    // return (2 * 2 + 3)    
    uint8_t program[] = {
        +instruction::def::PSH_LIT, 0, 2,
        +instruction::def::PSH_LIT, 0, 2,
        +instruction::def::MUL,
        +instruction::def::PSH_LIT, 0, 3,
        +instruction::def::ADD,
        +instruction::def::RET
    };
    
    uint16_t result = testProcessingUnit(program, sizeof(program));
    EXPECT_EQ(result, 7);
}

// ReturnStatment_Expression
TEST(VirtualMachineTest, OrderOfOperation_ParenthasesBeforeMul)
{
    // return (2 * (2 + 3)) should be 10    
    uint8_t program[] = {
        +instruction::def::PSH_LIT, 0, 2,
        +instruction::def::PSH_LIT, 0, 2,
        +instruction::def::PSH_LIT, 0, 3,
        +instruction::def::ADD,
        +instruction::def::MUL,
        +instruction::def::RET
    };
    
    uint16_t result = testProcessingUnit(program, sizeof(program));
    EXPECT_EQ(result, 10);
}

TEST(VirtualMachineTest, ReturnStatment_Default)
{
    // return 
    uint8_t program[] = {         
        +instruction::def::PSH_LIT, 0, 0,
        +instruction::def::RET }; // should return 0;
    
    uint16_t result = testProcessingUnit(program, sizeof(program));
    EXPECT_EQ(result, 0);
}

TEST(VirtualMachineTest, Return_ReturnNumericLiteral)
{
    // return 25
    uint8_t program[] = {
        +instruction::def::PSH_LIT, 0, 25,
        +instruction::def::RET
    };

    uint16_t result = testProcessingUnit(program, sizeof(program));
    EXPECT_EQ(result, 25);
}

TEST(VirtualMachineTest, Return_Variable)
{
    // let x = 25 
    // return x
    uint8_t program[] = {
        +instruction::def::PSH_LIT, 0, 25,
        +instruction::def::PEK_OFF, 0, 0,
        +instruction::def::RET
    };

    uint16_t result = testProcessingUnit(program, sizeof(program));
    EXPECT_EQ(result, 25);
}