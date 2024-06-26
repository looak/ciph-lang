#include <gtest/gtest.h>

#include <shared_defines.hpp>
#include "processing_unit.hpp"

using namespace ciph;

uint16_t testProcessingUnit(uint8_t* program, uint16_t size)
{
    ProcessingUnit unit;
    unit.load_program(program, size);
    return unit.execute();
}

TEST(ProcessingUnitTest, LoadAndExecuteSimpleProgram_Returns42)
{
    ProcessingUnit unit;
    //unit.reset();
    uint8_t program[] = { 
        +instruction::def::PSH_LIT, 0, 26,
        +instruction::def::PSH_LIT, 0, 16,
        +instruction::def::ADD,
        +instruction::def::POP_REG, +registers::def::ret,
        +instruction::def::RET};
    unit.load_program(program, sizeof(program));

    EXPECT_EQ(32, unit.registries()[+registers::def::pc]);
    int16_t result = unit.execute();
    EXPECT_EQ(42, result);
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
        +instruction::def::POP_REG, +registers::def::ret,
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
        +instruction::def::POP_REG, +registers::def::ret,
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
        +instruction::def::POP_REG, +registers::def::ret,
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
        +instruction::def::POP_REG, +registers::def::ret,
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
        +instruction::def::POP_REG, +registers::def::ret,
        +instruction::def::RET }; // should return 0;
    
    uint16_t result = testProcessingUnit(program, sizeof(program));
    EXPECT_EQ(result, 0);
}

TEST(VirtualMachineTest, Return_ReturnNumericLiteral)
{
    // return 25
    uint8_t program[] = {
        +instruction::def::PSH_LIT, 0, 25,
        +instruction::def::POP_REG, +registers::def::ret,
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
        +instruction::def::POP_REG, +registers::def::ret,
        +instruction::def::RET
    };

    uint16_t result = testProcessingUnit(program, sizeof(program));
    EXPECT_EQ(result, 25);
}

TEST(ProcessingUnitTest, LetStatement_MultipleVariables)
{
    // let x = 2
    // let y = 3
    // let z = x + y
    // return z + y
    uint8_t program[] = {   +instruction::def::PSH_LIT, 0, 2,
                            +instruction::def::PSH_LIT, 0, 3,
                            +instruction::def::PEK_OFF, +registers::def::sp, 0,
                            +instruction::def::PEK_OFF, +registers::def::sp, 1,
                            +instruction::def::ADD,
                            +instruction::def::PEK_OFF, +registers::def::sp, 2,
                            +instruction::def::PEK_OFF, +registers::def::sp, 1,
                            +instruction::def::ADD,
                            +instruction::def::POP_REG, +registers::def::ret,
                            +instruction::def::RET};

    uint16_t result = testProcessingUnit(program, sizeof(program));
    EXPECT_EQ(result, 8);
}

TEST(ProcessingUnitTest, Return_Equals)
{
    uint8_t program[] = {   +instruction::def::PSH_LIT, 0, 10,
                            +instruction::def::PEK_OFF, +registers::def::sp, 0,
                            +instruction::def::PSH_LIT, 0, 10,
                            +instruction::def::CMP, +registers::def::sp,
                            +instruction::def::MOV, +registers::def::ret, +registers::def::imm,
                            +instruction::def::RET};

    uint16_t result = testProcessingUnit(program, sizeof(program));
    EXPECT_EQ(result, 0); // no difference, should probably return 1.
}

TEST(ProcessingUnitTest, While) {
    // let x = 0
    // while ( x < 10) {
    //     x++
    // }
    // return x
    uint8_t program[] = {   +instruction::def::PSH_LIT, 0, 0,
                            +instruction::def::INC, +registers::def::sp, 0,
                            +instruction::def::PEK_OFF, +registers::def::sp, 0,
                            +instruction::def::PSH_LIT, 0, 10,                            
                            +instruction::def::CMP, +registers::def::sp,
                            +instruction::def::JLT, 0x00, 0x0E, // jump back fourteen bytes
                            +instruction::def::PEK_OFF, +registers::def::ret, 0,
                            +instruction::def::RET
                            };

    uint16_t result = testProcessingUnit(program, sizeof(program));
    EXPECT_EQ(result, 10);
}