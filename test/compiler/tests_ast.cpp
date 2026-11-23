#include <gtest/gtest.h>

#include <ciph/compiler/ast.hpp>

using namespace ciph;

// The packed bits on AST nodes: base fields every node has, and fields a
// subclass keeps above ASTBaseNode::kBaseBitsEnd.

TEST(ASTBitsTest, NewNodeComesFromTextAndHasNoSideEffect) {
	ASTNumericLiteralNode literal(SourceRange{}, 5);

	EXPECT_EQ(literal.readOrigin(), Origin::Text);
	EXPECT_FALSE(literal.readHasSideEffect());
}

TEST(ASTBitsTest, OriginCanBeChanged) {
	ASTNumericLiteralNode literal(SourceRange{}, 0);
	literal.setOrigin(Origin::Synthesized);

	EXPECT_EQ(literal.readOrigin(), Origin::Synthesized);
}

TEST(ASTBitsTest, PlainIdentifierHasNoIncDecAndNoSideEffect) {
	ASTIdentifierNode identifier(SourceRange{}, "x");

	EXPECT_EQ(identifier.readIncDec(), IncDec::None);
	EXPECT_FALSE(identifier.readHasSideEffect());
}

TEST(ASTBitsTest, IncrementAndDecrementHaveASideEffect) {
	ASTIdentifierNode increment(SourceRange{}, "x", IncDec::PostIncrement);
	ASTIdentifierNode decrement(SourceRange{}, "x", IncDec::PostDecrement);

	EXPECT_EQ(increment.readIncDec(), IncDec::PostIncrement);
	EXPECT_TRUE(increment.readHasSideEffect());
	EXPECT_EQ(decrement.readIncDec(), IncDec::PostDecrement);
	EXPECT_TRUE(decrement.readHasSideEffect());
}

TEST(ASTBitsTest, SubclassFieldsDoNotOverlapBaseFields) {
	ASTIdentifierNode identifier(SourceRange{}, "x", IncDec::PostDecrement);
	identifier.setOrigin(Origin::Synthesized);

	EXPECT_EQ(identifier.readIncDec(), IncDec::PostDecrement);
	EXPECT_TRUE(identifier.readHasSideEffect());
	EXPECT_EQ(identifier.readOrigin(), Origin::Synthesized);

	static_assert(ASTIdentifierNode::IncDecField::kShift >= ASTBaseNode::kBaseBitsEnd);
}

TEST(ASTBitsTest, CallHasASideEffect) {
	ASTCallNode call(SourceRange{}, "number");

	EXPECT_TRUE(call.readHasSideEffect());
}
