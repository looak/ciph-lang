#include <gtest/gtest.h>
#include <instructions.hpp>
#include <processing_unit.hpp>
#include <memory.hpp>

int16_t pop_word(uint8_t* bytecode, uint16_t& sp)
{
    int16_t value = 0;
    value = (value << 8) | bytecode[--sp];
    value = (value << 8) | bytecode[--sp];
    return value;
}

void write_word(uint8_t* bytecode, uint16_t& pc, int16_t value)
{
    bytecode[pc++] = value & u8(0xFF);
    bytecode[pc++] = (value >> 8) & u8(0xFF);
}

int16_t pop_helper(ExecutionContext& context)
{
    uint16_t& sp = context.registry[+registers::def::sp]; 
    auto value = pop_word(context.bytecode, sp);
    return value;
}

void push_helper(ExecutionContext& context, int16_t value)
{
    uint16_t& sp = context.registry[+registers::def::sp];
    write_word(context.bytecode, sp, value);
}

int16_t peek_word(uint8_t* bytecode, int16_t offset)
{
    int16_t value = 0;
    value = (value << 8) | bytecode[--offset];
    value = (value << 8) | bytecode[--offset];
    return value;
}

class InstructionsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        registries = mem.allocate(static_cast<uint16_t>(registers::def::reg_cnt));
        uint16_t addrs = mem.load(nullptr, 0);
        registries[+registers::def::pc] = addrs;
        registries[+registers::def::bp] = addrs;

        // setting up an arbitrary stack location,
        // 64 bytes from the start of the memory
        uint16_t stackAddrs = static_cast<uint16_t>(addrs + 64);
        registries[+registers::def::sp] = stackAddrs;
        registries[+registers::def::fp] = stackAddrs;
    }
public:
    Memory<0x100> mem;
    uint16_t* registries = nullptr;
};


TEST_F(InstructionsTest, PushHandler)
{
    uint8_t program[] = {
        +instruction::def::PSH_LIT, u8(0xf0), u8(0x0d)// push 0xf00d 
    };

    mem.load(program, sizeof(program));

    ExecutionContext context(registries, mem.getMemory());
    uint16_t pc = registries[+registers::def::pc];
    auto instruction = static_cast<instruction::def>(context.bytecode[pc]);

    EXPECT_EQ(instruction, instruction::def::PSH_LIT);

    instruction::handlers[instruction](context);
    int16_t value = peek_word(context.bytecode, registries[+registers::def::sp]);
	EXPECT_EQ(value, int16_t(0xf00d));
}

TEST_F(InstructionsTest, PeekHandler_Reg)
{
    uint8_t program[] = {
        +instruction::def::PSH_LIT, 0, 42,
        +instruction::def::PEK_REG, +registers::def::r0,
    };

    mem.load(program, sizeof(program));

    ExecutionContext context(registries, mem.getMemory());
    
    // run program
    uint16_t& pc = registries[+registers::def::pc];
    auto instruction = static_cast<instruction::def>(context.bytecode[pc]);    

    instruction::handlers[instruction](context);
    pc++;
    instruction = static_cast<instruction::def>(context.bytecode[pc]);
    EXPECT_EQ(instruction, instruction::def::PEK_REG);
    instruction::handlers[instruction](context);

    int16_t value = context.registry[+registers::def::r0];
    EXPECT_EQ(value, 42);

    value = pop_helper(context);
    EXPECT_EQ(value, 42);
}

TEST_F(InstructionsTest, PeekHandler_Offset)
{
    uint8_t program[] = {
        +instruction::def::PSH_LIT, 0, 42,
        +instruction::def::PSH_LIT, 0, 9,
        +instruction::def::PSH_LIT, 0, 17,
        +instruction::def::PSH_LIT, 0, 19,
        +instruction::def::PEK_OFF, +registers::def::r0, 0x02
    };

    mem.load(program, sizeof(program));

    ExecutionContext context(registries, mem.getMemory());
    
    // run program
    uint16_t& pc = registries[+registers::def::pc];
    for (int i = 0; i < 4; i++)
    {
        auto instruction = static_cast<instruction::def>(context.bytecode[pc]);
        instruction::handlers[instruction](context);
        pc++;
    }

    auto instruction = static_cast<instruction::def>(context.bytecode[pc]);
    EXPECT_EQ(instruction, instruction::def::PEK_OFF);
    instruction::handlers[instruction](context);

    int16_t value = context.registry[+registers::def::r0];
    EXPECT_EQ(value, 17);

    pop_helper(context); // pop twice
    value = pop_helper(context);
    EXPECT_EQ(value, 17);
}

int16_t BinaryExpressionHandlerTest(int16_t a, int16_t b, instruction::def _instr, InstructionsTest* fixture)
{
    int16_t offset = 0;
	ExecutionContext context(fixture->registries, fixture->mem.getMemory());
    uint8_t program[] = {
        +instruction::def::PSH_LIT, (a >> 8) & u8(0xFF), a & u8(0xFF),
        +instruction::def::PSH_LIT, (b >> 8) & u8(0xFF), b & u8(0xFF),
        +_instr
    };

    fixture->mem.load(program, sizeof(program));

    for (int i = 0; i < 3; i++)
    {
        uint16_t& pc = context.registry[+registers::def::pc];
        auto instr = static_cast<instruction::def>(context.bytecode[pc]);
		instruction::handlers[instr](context);
        pc++;
	}
	
	return pop_helper(context);
}

TEST_F(InstructionsTest, BinaryExpressionHandlersTests)
{
    using namespace instruction;
    std::vector<std::tuple<int16_t, int16_t, def, int16_t>> tests = {        
        std::make_tuple(7, 3, def::ADD, 10),
        std::make_tuple(9, 5, def::SUB, 4),
        std::make_tuple(3, 5, def::MUL, 15),
        std::make_tuple(9, 3, def::DIV, 3),
        // larger numbers
        std::make_tuple(164,       569,    def::MUL, 27780),
        std::make_tuple(32644,     16260,  def::SUB, 16384),
        std::make_tuple(16384,     32006,  def::SUB, -15622),
        std::make_tuple(1,         1,      def::SUB, 0),
        std::make_tuple(1,         -1,     def::SUB, 2),
        std::make_tuple(-1,        1,      def::ADD, 0),
        std::make_tuple(5559,      300,    def::DIV, 18)
    };

    for (auto& test : tests)
    {        
        auto [a, b, instruction, expected] = test;
        int16_t result = BinaryExpressionHandlerTest(a, b, instruction, this);
        EXPECT_EQ(result, expected);
    }
}

TEST_F(InstructionsTest, PopReg_ExpectValueInReturnRegister)
{
    uint8_t program[] = {
        +instruction::def::PSH_LIT, 0, 19,
        +instruction::def::POP_REG, +registers::def::ret
    };

    mem.load(program, sizeof(program));

    ExecutionContext context(registries, mem.getMemory());
    
    // tick program
    uint16_t& pc = registries[+registers::def::pc];
    auto instruction = static_cast<instruction::def>(context.bytecode[pc]);
    instruction::handlers[instruction](context);

    instruction = static_cast<instruction::def>(context.bytecode[++pc]);
    EXPECT_EQ(instruction, instruction::def::POP_REG);
    instruction::handlers[instruction](context);

    int16_t value = context.registry[+registers::def::ret];
    EXPECT_EQ(value, 19);
}

