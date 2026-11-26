// Tests for diagnostics: what one holds, how it is shown as text, and what
// each stage of the compiler reports for a real mistake.
//
// These are written ahead of the code. They expect:
//
//   struct TextSpan { Position start; Position end; };
//   struct Location { TextSpan span; };
//   struct Diagnostic { ErrorCode code; Location location; std::string message; };
//
//   all_error_codes                      every code from CIPH_DIAGNOSTICS, to loop over
//   describe(ErrorCode)                  the code's fixed text, as a std::string_view
//   diag::undeclaredIdentifier(location, name)
//   locationOf(node, tokens)             where an AST node came from
//   formatDiagnostic(sourceName, diagnostic)
//
// A span is half-open: `end` is one column past the last character, so a
// one-character token at column 10 is 1:10 to 1:11. `start == end` is an
// empty span: a point between characters, or no place at all.

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include <ciph/compiler/ast.hpp>
#include <ciph/compiler/compile_unit.hpp>
#include <ciph/compiler/diagnostics.hpp>
#include <ciph/compiler/lexer.hpp>

using namespace ciph;

namespace {

// Runs every stage on `source`, each only if the one before passed, and
// returns what they reported.
std::vector<Diagnostic>
diagnose(std::string source) {
    CompileUnit unit(std::move(source));
    if (unit.parse() && unit.semanticAnalyze()) {
        unit.generateCode();
    }
    return unit.readDiagnostics();
}

Location
at(uint32_t startLine, uint32_t startColumn, uint32_t endLine, uint32_t endColumn) {
    return Location{.span = {.start = {startLine, startColumn}, .end = {endLine, endColumn}}};
}

void
expectSpan(const Location& location, uint32_t startLine, uint32_t startColumn, uint32_t endLine, uint32_t endColumn) {
    EXPECT_EQ(location.span.start.line, startLine);
    EXPECT_EQ(location.span.start.column, startColumn);
    EXPECT_EQ(location.span.end.line, endLine);
    EXPECT_EQ(location.span.end.column, endColumn);
}

void
expectEmptySpan(const Location& location) {
    EXPECT_EQ(location.span.start.line, location.span.end.line);
    EXPECT_EQ(location.span.start.column, location.span.end.column);
}

bool
contains(std::string_view text, std::string_view part) {
    return text.find(part) != std::string_view::npos;
}

// The first statement of a parsed program.
const ASTBaseNode*
firstStatement(const CompileUnit& unit) {
    const auto* program = static_cast<const ASTProgramNode*>(unit.astRoot());
    return program->readStatements().front();
}

} // namespace

// ---- The diagnostic type on its own ----

TEST(DiagnosticTest, EveryCodeHasADescription) {
    for (const ErrorCode code : all_error_codes) {
        EXPECT_FALSE(describe(code).empty()) << "code 0x" << std::hex << static_cast<uint16_t>(code);
    }
}

TEST(DiagnosticTest, FactoryFillsCodeLocationAndMessage) {
    const Diagnostic diagnostic = diag::undeclaredIdentifier(at(3, 5, 3, 10), "total");

    EXPECT_EQ(diagnostic.code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
    expectSpan(diagnostic.location, 3, 5, 3, 10);
    EXPECT_TRUE(contains(diagnostic.message, "total")) << diagnostic.message;
}

TEST(DiagnosticTest, DefaultLocationIsEmpty) {
    const Diagnostic diagnostic{.code = ErrorCode::SYNTAX_ERROR, .location = {}, .message = ""};

    expectEmptySpan(diagnostic.location);
}

// ---- Shown as text ----

TEST(FormatDiagnosticTest, StartsWithSourceNameLineAndColumn) {
    const Diagnostic diagnostic{.code = ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER,
                                .location = at(1, 10, 1, 11),
                                .message = "'x' is not declared at this point"};

    const std::string text = formatDiagnostic("script.ciph", diagnostic);

    EXPECT_EQ(text.rfind("script.ciph:1:10: error: ", 0), 0u) << text;
}

TEST(FormatDiagnosticTest, IncludesCodeDescriptionAndMessage) {
    const Diagnostic diagnostic{.code = ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER,
                                .location = at(1, 10, 1, 11),
                                .message = "'x' is not declared at this point"};

    const std::string text = formatDiagnostic("script.ciph", diagnostic);

    EXPECT_TRUE(contains(text, "e201")) << text;
    EXPECT_TRUE(contains(text, describe(ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER))) << text;
    EXPECT_TRUE(contains(text, "'x' is not declared at this point")) << text;
}

TEST(FormatDiagnosticTest, EndsWithANewline) {
    const Diagnostic diagnostic{.code = ErrorCode::SYNTAX_ERROR, .location = at(1, 1, 1, 2), .message = "anything"};

    const std::string text = formatDiagnostic("script.ciph", diagnostic);

    ASSERT_FALSE(text.empty());
    EXPECT_EQ(text.back(), '\n');
}

TEST(FormatDiagnosticTest, UnknownCodeIsShownAsItsNumber) {
    const Diagnostic diagnostic{.code = static_cast<ErrorCode>(0x1234), .location = at(1, 1, 1, 2), .message = ""};

    const std::string text = formatDiagnostic("script.ciph", diagnostic);

    EXPECT_TRUE(contains(text, "1234")) << text;
}

// ---- Where an AST node came from ----

TEST(LocationOfTest, NodeBuiltByHand_HasAnEmptyLocation) {
    const ASTNumericLiteralNode node(SourceRange{}, 1);

    expectEmptySpan(locationOf(node, {}));
}

// A node with a focus token points at that token: a let at its name.
TEST(LocationOfTest, NodeWithFocus_CoversTheFocusToken) {
    CompileUnit unit("let total = 1 + 2;");
    ASSERT_TRUE(unit.parse());

    expectSpan(locationOf(*firstStatement(unit), unit.readTokens()), 1, 5, 1, 10);
}

// A node without one covers everything it was built from.
TEST(LocationOfTest, NodeWithoutFocus_CoversFirstTokenToLast) {
    CompileUnit unit("return 1 + 2;");
    ASSERT_TRUE(unit.parse());

    expectSpan(locationOf(*firstStatement(unit), unit.readTokens()), 1, 1, 1, 13);
}

// ---- Real mistakes, one stage at a time ----

TEST(DiagnosticsFromLexerTest, UnknownCharacter_PointsAtTheCharacter) {
    auto diagnostics = diagnose("return x $ 3;");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].code, ErrorCode::LEXER_UNKNOWN_CHARACTER);
    expectSpan(diagnostics[0].location, 1, 10, 1, 11);
    EXPECT_TRUE(contains(diagnostics[0].message, "$")) << diagnostics[0].message;
}

// The place a ';' belongs is a point, not a character: an empty span just
// past the statement's last token.
TEST(DiagnosticsFromParserTest, MissingSemicolon_PointsJustPastTheStatement) {
    auto diagnostics = diagnose("let x = 1");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].code, ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON);
    expectSpan(diagnostics[0].location, 1, 10, 1, 10);
}

// Used to read past the last token and stop the whole compiler.
TEST(DiagnosticsFromParserTest, FunctionCutOffAfterItsName_IsAnError) {
    auto diagnostics = diagnose("fn f");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].location.span.start.line, 1u);
}

TEST(DiagnosticsFromParserTest, FunctionWithoutParentheses_IsReportedAtTheBrace) {
    auto diagnostics = diagnose("fn f { return 1; }");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].code, ErrorCode::SYNTAX_ERROR_PARENTHESIS_MISSMATCH);
    expectSpan(diagnostics[0].location, 1, 6, 1, 7);
}

// Used to compile, with the value cut down to 16 bits.
TEST(DiagnosticsFromParserTest, NumberTooLarge_PointsAtTheNumber) {
    auto diagnostics = diagnose("return 70000;");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].code, ErrorCode::SYNTAX_ERROR_NUMBER_OUT_OF_RANGE);
    expectSpan(diagnostics[0].location, 1, 8, 1, 13);
}

TEST(DiagnosticsFromParserTest, NumberFarTooLarge_IsTheSameError) {
    auto diagnostics = diagnose("return 99999999999;");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].code, ErrorCode::SYNTAX_ERROR_NUMBER_OUT_OF_RANGE);
}

TEST(DiagnosticsFromAnalyzerTest, UndeclaredName_PointsAtTheName) {
    auto diagnostics = diagnose("return y;");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].code, ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER);
    expectSpan(diagnostics[0].location, 1, 8, 1, 9);
    EXPECT_TRUE(contains(diagnostics[0].message, "y")) << diagnostics[0].message;
}

TEST(DiagnosticsFromAnalyzerTest, RedeclaredName_PointsAtTheSecondDeclaration) {
    auto diagnostics = diagnose("let x = 1; let x = 2;");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].code, ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER);
    expectSpan(diagnostics[0].location, 1, 16, 1, 17);
    EXPECT_TRUE(contains(diagnostics[0].message, "x")) << diagnostics[0].message;
}

TEST(DiagnosticsFromCodeGeneratorTest, Function_IsNotSupportedYet_PointsAtItsName) {
    auto diagnostics = diagnose("fn f() { return 1; } return 0;");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].code, ErrorCode::CODEGEN_ERROR_UNSUPPORTED);
    expectSpan(diagnostics[0].location, 1, 4, 1, 5);
}

// ---- Across stages ----

TEST(DiagnosticsAcrossStagesTest, ValidProgram_ReportsNothing) {
    EXPECT_TRUE(diagnose("let x = 1; return x;").empty());
}

// `y` is undeclared too, but the analyzer never runs on a program that did
// not parse.
TEST(DiagnosticsAcrossStagesTest, SyntaxError_StopsBeforeTheAnalyzer) {
    auto diagnostics = diagnose("let x = 1 return y;");

    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics[0].code, ErrorCode::SYNTAX_ERROR_EXPECTED_SEMICOLON);
}

TEST(DiagnosticsAcrossStagesTest, SeveralErrors_ComeInSourceOrder) {
    auto diagnostics = diagnose("return a;\nreturn b;");

    ASSERT_EQ(diagnostics.size(), 2u);
    expectSpan(diagnostics[0].location, 1, 8, 1, 9);
    expectSpan(diagnostics[1].location, 2, 8, 2, 9);
}

TEST(DiagnosticsAcrossStagesTest, WindowsLineEndings_DoNotShiftTheColumn) {
    auto diagnostics = diagnose("let x = 1;\r\nreturn y;");

    ASSERT_EQ(diagnostics.size(), 1u);
    expectSpan(diagnostics[0].location, 2, 8, 2, 9);
}
