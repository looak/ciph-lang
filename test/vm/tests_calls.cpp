// VM tests for function calls: CALL, RET, and reading a frame's slots.
//
// The calling convention these tests describe:
//
//   - The caller pushes the arguments, first argument first.
//   - CALL address  pushes the return address, pushes fp, sets fp = sp and
//                   jumps to the address (counted from the program's start,
//                   like JMP).
//   - RET           sets sp = fp, pops fp, pops the return address and
//                   continues there. It ends nothing; HALT ends the program.
//   - The result travels in the `ret` register.
//   - The caller removes the arguments after the call returns.
//   - A program starts with `CALL main; HALT`.
//
// A frame, for a function with two parameters. The stack grows upward, and a
// slot is two bytes:
//
//   slot -4   argument 0
//   slot -3   argument 1
//   slot -2   return address     pushed by CALL
//   slot -1   caller's fp        pushed by CALL
//   slot  0   local 0            <- fp
//   slot  1   local 1
//
// So argument i of n sits in slot (i - n - 2), and PEK_OFF, INC and DEC take
// a signed slot number.
//
// Each program is written with the address of every instruction in front,
// because CALL takes an address.

#include <gtest/gtest.h>

#include <ciph/shared/shared_defines.hpp>
#include <ciph/vm/processing_unit.hpp>

using namespace ciph;

namespace {

// A slot number as the byte PEK_OFF, INC and DEC take. Negative slots are
// the linkage and the arguments.
constexpr uint8_t
slot(int number) {
    return static_cast<uint8_t>(static_cast<int8_t>(number));
}

// A scratch register for dropping an argument: `POP_REG, scratch`.
const uint8_t scratch = +registers::def::r0;

struct RunResult {
    int16_t ret;        // the `ret` register when the program stopped
    uint16_t sp;        // where the stack ended up
    uint16_t fp;
    uint16_t stackBase; // where sp and fp started
};

RunResult
run(uint8_t* program, uint16_t size) {
    ProcessingUnit unit;
    unit.load_program(program, size);
    const uint16_t stackBase = unit.registries()[+registers::def::sp];

    unit.execute();

    return RunResult{.ret = static_cast<int16_t>(unit.registries()[+registers::def::ret]),
                     .sp = unit.registries()[+registers::def::sp],
                     .fp = unit.registries()[+registers::def::fp],
                     .stackBase = stackBase};
}

// Every call has returned and every argument has been dropped: the stack is
// empty again and fp is back where it started.
void
expectStackUnwound(const RunResult& result) {
    EXPECT_EQ(result.sp, result.stackBase) << "values were left on the stack, or too many were removed";
    EXPECT_EQ(result.fp, result.stackBase) << "fp was not restored";
}

} // namespace

TEST(CallTest, CallMain_ReturnReachesTheHalt) {
    // return 42;
    uint8_t program[] = {
        /*  0 */ +instruction::def::CALL, 0, 4, // main
        /*  3 */ +instruction::def::HALT,
        // main
        /*  4 */ +instruction::def::PSH_LIT, 0, 42,
        /*  7 */ +instruction::def::POP_REG, +registers::def::ret,
        /*  9 */ +instruction::def::RET,
    };

    RunResult result = run(program, sizeof(program));

    EXPECT_EQ(result.ret, 42);
    expectStackUnwound(result);
}

TEST(CallTest, Return_ContinuesAfterTheCall) {
    // fn f() { return 41; }
    // return f() + 1;
    uint8_t program[] = {
        /*  0 */ +instruction::def::CALL, 0, 4, // main
        /*  3 */ +instruction::def::HALT,
        // main
        /*  4 */ +instruction::def::CALL, 0, 16, // f
        /*  7 */ +instruction::def::PSH_REG, +registers::def::ret,
        /*  9 */ +instruction::def::PSH_LIT, 0, 1,
        /* 12 */ +instruction::def::ADD,
        /* 13 */ +instruction::def::POP_REG, +registers::def::ret,
        /* 15 */ +instruction::def::RET,
        // f
        /* 16 */ +instruction::def::PSH_LIT, 0, 41,
        /* 19 */ +instruction::def::POP_REG, +registers::def::ret,
        /* 21 */ +instruction::def::RET,
    };

    RunResult result = run(program, sizeof(program));

    EXPECT_EQ(result.ret, 42);
    expectStackUnwound(result);
}

TEST(CallTest, SameFunctionCalledTwice) {
    // fn f() { return 21; }
    // return f() + f();
    uint8_t program[] = {
        /*  0 */ +instruction::def::CALL, 0, 4, // main
        /*  3 */ +instruction::def::HALT,
        // main
        /*  4 */ +instruction::def::CALL, 0, 18, // f
        /*  7 */ +instruction::def::PSH_REG, +registers::def::ret,
        /*  9 */ +instruction::def::CALL, 0, 18, // f
        /* 12 */ +instruction::def::PSH_REG, +registers::def::ret,
        /* 14 */ +instruction::def::ADD,
        /* 15 */ +instruction::def::POP_REG, +registers::def::ret,
        /* 17 */ +instruction::def::RET,
        // f
        /* 18 */ +instruction::def::PSH_LIT, 0, 21,
        /* 21 */ +instruction::def::POP_REG, +registers::def::ret,
        /* 23 */ +instruction::def::RET,
    };

    RunResult result = run(program, sizeof(program));

    EXPECT_EQ(result.ret, 42);
    expectStackUnwound(result);
}

TEST(CallTest, EachFrameHasItsOwnLocals) {
    // fn f() { let y = 5; return y; }
    // let x = 10;
    // return f() + x;
    //
    // Both x and y are local 0 of their own frame. After the call, slot 0
    // has to be x again.
    uint8_t program[] = {
        /*  0 */ +instruction::def::CALL, 0, 4, // main
        /*  3 */ +instruction::def::HALT,
        // main
        /*  4 */ +instruction::def::PSH_LIT, 0, 10, // let x = 10
        /*  7 */ +instruction::def::CALL, 0, 19, // f
        /* 10 */ +instruction::def::PSH_REG, +registers::def::ret,
        /* 12 */ +instruction::def::PEK_OFF, +registers::def::sp, slot(0), // x
        /* 15 */ +instruction::def::ADD,
        /* 16 */ +instruction::def::POP_REG, +registers::def::ret,
        /* 18 */ +instruction::def::RET,
        // f
        /* 19 */ +instruction::def::PSH_LIT, 0, 5, // let y = 5
        /* 22 */ +instruction::def::PEK_OFF, +registers::def::ret, slot(0), // return y
        /* 25 */ +instruction::def::RET,
    };

    RunResult result = run(program, sizeof(program));

    EXPECT_EQ(result.ret, 15);
    expectStackUnwound(result);
}

TEST(CallTest, Return_DiscardsTheLocalsOfTheFrame) {
    // fn f() { let a = 1; let b = 2; let c = 3; return 7; }
    // return f();
    //
    // f leaves three locals on the stack. RET has to drop them, or it would
    // take one of them for the return address.
    uint8_t program[] = {
        /*  0 */ +instruction::def::CALL, 0, 4, // main
        /*  3 */ +instruction::def::HALT,
        // main
        /*  4 */ +instruction::def::CALL, 0, 8, // f
        /*  7 */ +instruction::def::RET,
        // f
        /*  8 */ +instruction::def::PSH_LIT, 0, 1,
        /* 11 */ +instruction::def::PSH_LIT, 0, 2,
        /* 14 */ +instruction::def::PSH_LIT, 0, 3,
        /* 17 */ +instruction::def::PSH_LIT, 0, 7,
        /* 20 */ +instruction::def::POP_REG, +registers::def::ret,
        /* 22 */ +instruction::def::RET,
    };

    RunResult result = run(program, sizeof(program));

    EXPECT_EQ(result.ret, 7);
    expectStackUnwound(result);
}

TEST(CallTest, Argument_IsReadAtANegativeSlot) {
    // fn id(a) { return a; }
    // return id(7);
    uint8_t program[] = {
        /*  0 */ +instruction::def::CALL, 0, 4, // main
        /*  3 */ +instruction::def::HALT,
        // main
        /*  4 */ +instruction::def::PSH_LIT, 0, 7, // argument a
        /*  7 */ +instruction::def::CALL, 0, 13, // id
        /* 10 */ +instruction::def::POP_REG, scratch, // the caller drops the argument
        /* 12 */ +instruction::def::RET,
        // id: one parameter, so a is slot 0 - 1 - 2 = -3
        /* 13 */ +instruction::def::PEK_OFF, +registers::def::ret, slot(-3),
        /* 16 */ +instruction::def::RET,
    };

    RunResult result = run(program, sizeof(program));

    EXPECT_EQ(result.ret, 7);
    expectStackUnwound(result);
}

TEST(CallTest, Arguments_KeepTheirOrder) {
    // fn sub(a, b) { return a - b; }
    // return sub(10, 4);
    //
    // Subtraction tells the two arguments apart: swapped, the result is -6.
    uint8_t program[] = {
        /*  0 */ +instruction::def::CALL, 0, 4, // main
        /*  3 */ +instruction::def::HALT,
        // main
        /*  4 */ +instruction::def::PSH_LIT, 0, 10, // argument a
        /*  7 */ +instruction::def::PSH_LIT, 0, 4, // argument b
        /* 10 */ +instruction::def::CALL, 0, 18, // sub
        /* 13 */ +instruction::def::POP_REG, scratch,
        /* 15 */ +instruction::def::POP_REG, scratch,
        /* 17 */ +instruction::def::RET,
        // sub: two parameters, so a is slot -4 and b is slot -3
        /* 18 */ +instruction::def::PEK_OFF, +registers::def::sp, slot(-4),
        /* 21 */ +instruction::def::PEK_OFF, +registers::def::sp, slot(-3),
        /* 24 */ +instruction::def::SUB,
        /* 25 */ +instruction::def::POP_REG, +registers::def::ret,
        /* 27 */ +instruction::def::RET,
    };

    RunResult result = run(program, sizeof(program));

    EXPECT_EQ(result.ret, 6);
    expectStackUnwound(result);
}

TEST(CallTest, ArgumentsAndLocals_InOneFrame) {
    // fn f(a) { let y = 3; return a * y; }
    // return f(5);
    uint8_t program[] = {
        /*  0 */ +instruction::def::CALL, 0, 4, // main
        /*  3 */ +instruction::def::HALT,
        // main
        /*  4 */ +instruction::def::PSH_LIT, 0, 5, // argument a
        /*  7 */ +instruction::def::CALL, 0, 13, // f
        /* 10 */ +instruction::def::POP_REG, scratch,
        /* 12 */ +instruction::def::RET,
        // f
        /* 13 */ +instruction::def::PSH_LIT, 0, 3, // let y = 3
        /* 16 */ +instruction::def::PEK_OFF, +registers::def::sp, slot(-3), // a
        /* 19 */ +instruction::def::PEK_OFF, +registers::def::sp, slot(0), // y
        /* 22 */ +instruction::def::MUL,
        /* 23 */ +instruction::def::POP_REG, +registers::def::ret,
        /* 25 */ +instruction::def::RET,
    };

    RunResult result = run(program, sizeof(program));

    EXPECT_EQ(result.ret, 15);
    expectStackUnwound(result);
}

TEST(CallTest, Decrement_OnAnArgument) {
    // fn f(n) { n--; return n; }
    // return f(5);
    //
    // INC and DEC address a slot the same way PEK_OFF does, so they take a
    // negative slot too.
    uint8_t program[] = {
        /*  0 */ +instruction::def::CALL, 0, 4, // main
        /*  3 */ +instruction::def::HALT,
        // main
        /*  4 */ +instruction::def::PSH_LIT, 0, 5, // argument n
        /*  7 */ +instruction::def::CALL, 0, 13, // f
        /* 10 */ +instruction::def::POP_REG, scratch,
        /* 12 */ +instruction::def::RET,
        // f
        /* 13 */ +instruction::def::DEC, +registers::def::sp, slot(-3),
        /* 16 */ +instruction::def::PEK_OFF, +registers::def::ret, slot(-3),
        /* 19 */ +instruction::def::RET,
    };

    RunResult result = run(program, sizeof(program));

    EXPECT_EQ(result.ret, 4);
    expectStackUnwound(result);
}

TEST(CallTest, NestedCalls_ReturnOneLevelAtATime) {
    // fn g() { return 41; }
    // fn f() { let one = 1; return g() + one; }
    // return f();
    uint8_t program[] = {
        /*  0 */ +instruction::def::CALL, 0, 4, // main
        /*  3 */ +instruction::def::HALT,
        // main
        /*  4 */ +instruction::def::CALL, 0, 8, // f
        /*  7 */ +instruction::def::RET,
        // f
        /*  8 */ +instruction::def::PSH_LIT, 0, 1, // let one = 1
        /* 11 */ +instruction::def::CALL, 0, 23, // g
        /* 14 */ +instruction::def::PSH_REG, +registers::def::ret,
        /* 16 */ +instruction::def::PEK_OFF, +registers::def::sp, slot(0), // one
        /* 19 */ +instruction::def::ADD,
        /* 20 */ +instruction::def::POP_REG, +registers::def::ret,
        /* 22 */ +instruction::def::RET,
        // g
        /* 23 */ +instruction::def::PSH_LIT, 0, 41,
        /* 26 */ +instruction::def::POP_REG, +registers::def::ret,
        /* 28 */ +instruction::def::RET,
    };

    RunResult result = run(program, sizeof(program));

    EXPECT_EQ(result.ret, 42);
    expectStackUnwound(result);
}

TEST(CallTest, Recursion_SumToN) {
    // fn sum(n) { if n == 0 return 0; return n + sum(n - 1); }
    // return sum(3);
    //
    // There is no `if` yet and a conditional jump only goes backward, so the
    // `return 0` block sits in front of the function and sum jumps back to it.
    uint8_t program[] = {
        /*  0 */ +instruction::def::CALL, 0, 42, // main
        /*  3 */ +instruction::def::HALT,
        // return 0: part of sum, reached by the jump at 18
        /*  4 */ +instruction::def::PSH_LIT, 0, 0,
        /*  7 */ +instruction::def::POP_REG, +registers::def::ret,
        /*  9 */ +instruction::def::RET,
        // sum: one parameter, n is slot -3
        /* 10 */ +instruction::def::PEK_OFF, +registers::def::sp, slot(-3), // n
        /* 13 */ +instruction::def::PSH_LIT, 0, 0,
        /* 16 */ +instruction::def::CMP, +registers::def::sp, // imm = n - 0
        /* 18 */ +instruction::def::JEQ, 0, 17, // back to 4: (18 + 3) - 4
        /* 21 */ +instruction::def::PEK_OFF, +registers::def::sp, slot(-3), // n
        /* 24 */ +instruction::def::PSH_LIT, 0, 1,
        /* 27 */ +instruction::def::SUB, // argument: n - 1
        /* 28 */ +instruction::def::CALL, 0, 10, // sum
        /* 31 */ +instruction::def::POP_REG, scratch, // drop the argument
        /* 33 */ +instruction::def::PSH_REG, +registers::def::ret, // sum(n - 1)
        /* 35 */ +instruction::def::PEK_OFF, +registers::def::sp, slot(-3), // n
        /* 38 */ +instruction::def::ADD,
        /* 39 */ +instruction::def::POP_REG, +registers::def::ret,
        /* 41 */ +instruction::def::RET,
        // main
        /* 42 */ +instruction::def::PSH_LIT, 0, 3, // argument n
        /* 45 */ +instruction::def::CALL, 0, 10, // sum
        /* 48 */ +instruction::def::POP_REG, scratch,
        /* 50 */ +instruction::def::RET,
    };

    RunResult result = run(program, sizeof(program));

    EXPECT_EQ(result.ret, 6); // 3 + 2 + 1 + 0
    expectStackUnwound(result);
}
