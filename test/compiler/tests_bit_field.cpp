#include <gtest/gtest.h>

#include <ciph/compiler/bit_field.hpp>

using namespace ciph;

namespace {

enum class Colour : uint8_t { Red, Green, Blue, White };

using Flag = BitField<bool, 0, 1>;
using Paint = Flag::Next<Colour, 2>;
using Count = Paint::Next<uint8_t, 8>;
using Top = BitField<uint8_t, 60, 4>;

} // namespace

TEST(BitFieldTest, NextStartsWhereThePreviousFieldEnds) {
	EXPECT_EQ(Flag::kShift, 0u);
	EXPECT_EQ(Paint::kShift, 1u);
	EXPECT_EQ(Count::kShift, 3u);
	EXPECT_EQ(Count::kEnd, 11u);
}

TEST(BitFieldTest, ReadsBackWhatWasWritten) {
	uint64_t bits = 0;
	bits = Flag::set(bits, true);
	bits = Paint::set(bits, Colour::Blue);
	bits = Count::set(bits, 200);

	EXPECT_TRUE(Flag::get(bits));
	EXPECT_EQ(Paint::get(bits), Colour::Blue);
	EXPECT_EQ(Count::get(bits), 200);
}

TEST(BitFieldTest, WritingOneFieldLeavesItsNeighboursAlone) {
	uint64_t bits = 0;
	bits = Flag::set(bits, true);
	bits = Count::set(bits, 0xFF);

	bits = Paint::set(bits, Colour::White);
	bits = Paint::set(bits, Colour::Red);

	EXPECT_TRUE(Flag::get(bits));
	EXPECT_EQ(Paint::get(bits), Colour::Red);
	EXPECT_EQ(Count::get(bits), 0xFF);
}

TEST(BitFieldTest, OverwritingClearsTheOldValue) {
	uint64_t bits = Count::set(0, 0xFF);
	bits = Count::set(bits, 1);
	EXPECT_EQ(Count::get(bits), 1);

	bits = Flag::set(Flag::set(0, true), false);
	EXPECT_FALSE(Flag::get(bits));
}

TEST(BitFieldTest, WorksAtTheTopOfTheWord) {
	uint64_t bits = Top::set(0, 0xF);
	EXPECT_EQ(Top::get(bits), 0xF);
	EXPECT_EQ(bits, 0xF000'0000'0000'0000ull);
}
