#include <gtest/gtest.h>

#include "shared_defines.hpp"
#include "processing_unit.hpp"

TEST(ProcessingUnitTest, MustStartSomeWhere)
{
    ProcessingUnit unit;
    unit.reset();
    unit.load_program(nullptr, 0);
    unit.execute();
    ASSERT_TRUE(true);
}

TEST(ProcessingUnitTest, LoadSimpleProgram)
{
    ProcessingUnit unit;
    unit.reset();
    uint8_t program[] = { 
        +Instructions::PUSH,  };
    unit.load_program(program, sizeof(program));
    unit.execute();
    ASSERT_TRUE(true);
}
