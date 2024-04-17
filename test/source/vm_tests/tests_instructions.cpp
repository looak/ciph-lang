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
    uint16_t& sp = context.registry[+Register::sp]; 
    auto value = pop_word(context.bytecode, sp);
    return value;
}

void push_helper(ExecutionContext& context, int16_t value)
{
    uint16_t& sp = context.registry[+Register::sp];
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
        registries = mem.allocate(static_cast<uint16_t>(Register::reg_cnt));
        uint16_t addrs = mem.load(nullptr, 0);
        registries[+Register::pc] = addrs;
        registries[+Register::bp] = addrs;

        // setting up an arbitrary stack location,
        // 64 bytes from the start of the memory
        uint16_t stackAddrs = static_cast<uint16_t>(addrs + 64);
        registries[+Register::sp] = stackAddrs;
        registries[+Register::fp] = stackAddrs;
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
    uint16_t pc = registries[+Register::pc];
    auto instruction = static_cast<instruction::def>(context.bytecode[pc]);

    EXPECT_EQ(instruction, instruction::def::PSH_LIT);

    instruction::handlers[instruction](context);
    int16_t value = peek_word(context.bytecode, registries[+Register::sp]);
	EXPECT_EQ(value, int16_t(0xf00d));
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
        uint16_t& pc = context.registry[+Register::pc];
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

// TEST(InstructionsTest, SubHandler)
// {
//     int offset = 0;
// 	ExecutionContext context;
//     context.bytecode = new uint8_t[11];
// 	helper_push(context, offset, 10);
//     helper_push(context, ++offset, 20);
// 	context.bytecode[++offset] = +Instructions::SUB;

//     for (int i = 0; i < 3; i++)
//     {
//         auto instruction = static_cast<Instructions>(context.bytecode[context.pc]);
// 		instruction::handlers[instruction](context);
//         context.pc++;
// 	}
	
// 	EXPECT_EQ(context.stack.back(), -10);
// }

// TEST(InstructionsTest, PeekHandler)
// {
//     int offset = 0;
//     ExecutionContext context;
//     context.bytecode = new uint8_t[4];
//     write_value(context, offset, 1);
//     context.stack.push_back(10);
//     context.stack.push_back(20);
//     context.stack.push_back(30);

//     instruction::peek_handler(context);
//     EXPECT_EQ(context.stack.back(), 20);
//     EXPECT_EQ(context.stack.size(), 4);
// }