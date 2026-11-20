#include <gtest/gtest.h>

#include <ciph/platform/windows_command_line.hpp>

using namespace ciph::platform;

TEST(WindowsCommandLineTest, PlainArgument_IsLeftAlone) {
    EXPECT_EQ(quote_windows_argument("out.bin"), "out.bin");
    EXPECT_EQ(quote_windows_argument("-o"), "-o");
}

TEST(WindowsCommandLineTest, BackslashesWithoutQuotes_AreLeftAlone) {
    // Backslashes only matter to the parser when a quote follows them.
    EXPECT_EQ(quote_windows_argument(R"(C:\dev\file.ciph)"), R"(C:\dev\file.ciph)");
}

TEST(WindowsCommandLineTest, EmptyArgument_IsQuoted) {
    // Without quotes an empty argument would vanish from the command line.
    EXPECT_EQ(quote_windows_argument(""), R"("")");
}

TEST(WindowsCommandLineTest, ArgumentWithSpace_IsQuoted) {
    EXPECT_EQ(quote_windows_argument("hello world"), R"("hello world")");
}

TEST(WindowsCommandLineTest, EmbeddedQuote_IsEscaped) {
    EXPECT_EQ(quote_windows_argument(R"(say "hi")"), R"("say \"hi\"")");
}

TEST(WindowsCommandLineTest, BackslashesBeforeQuote_AreDoubledAndQuoteEscaped) {
    // a\"b : one backslash before a quote becomes 2*1+1 = 3 backslashes.
    EXPECT_EQ(quote_windows_argument(R"(a\"b)"), R"("a\\\"b")");
}

TEST(WindowsCommandLineTest, TrailingBackslashInQuotedArgument_IsDoubled) {
    // Otherwise the backslash would escape the closing quote.
    EXPECT_EQ(quote_windows_argument(R"(C:\my dir\)"), R"("C:\my dir\\")");
}

TEST(WindowsCommandLineTest, CommandLine_JoinsExecutableAndArguments) {
    EXPECT_EQ(windows_command_line(R"(C:\tools\ciph-compiler.exe)", {"main.ciph", "-o", "out.bin"}),
              R"(C:\tools\ciph-compiler.exe main.ciph -o out.bin)");
}

TEST(WindowsCommandLineTest, CommandLine_QuotesExecutableWithSpaces) {
    EXPECT_EQ(windows_command_line(R"(C:\Program Files\ciph\ciph-vm.exe)", {"my program.bin"}),
              R"("C:\Program Files\ciph\ciph-vm.exe" "my program.bin")");
}
