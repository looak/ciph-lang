#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <ciph/platform/process.hpp>

using namespace ciph::platform;

namespace {

// Path of the helper program, baked in by CMake (see test/CMakeLists.txt).
std::filesystem::path
child_path() {
    return std::filesystem::path(CIPH_TEST_CHILD_PATH).make_preferred();
}

// A file path that is unique to the running test, inside gtest's temp folder.
std::filesystem::path
scratch_file(const std::string& name) {
    const auto* test = ::testing::UnitTest::GetInstance()->current_test_info();
    return std::filesystem::path(::testing::TempDir()) /
           (std::string("ciph_") + test->test_suite_name() + "_" + test->name() + "_" + name);
}

std::string
read_file(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

// Starts the helper; fails the test with the error message if it did not start.
Process
start_child(std::vector<std::string> arguments, std::filesystem::path working_directory = {},
            ConsoleMode console = ConsoleMode::none) {
    auto result = Process::start({.executable = child_path(),
                                  .arguments = std::move(arguments),
                                  .working_directory = std::move(working_directory),
                                  .console = console});
    if (!result) {
        // gtest reports an exception thrown by a test as a failure.
        throw std::runtime_error(result.error().message);
    }
    return std::move(*result);
}

} // namespace

TEST(ProcessTest, Wait_ReturnsExitCode) {
    EXPECT_EQ(start_child({"exit", "0"}).wait(), 0);
    EXPECT_EQ(start_child({"exit", "3"}).wait(), 3);
    EXPECT_EQ(start_child({"exit", "42"}).wait(), 42);
}

TEST(ProcessTest, Wait_CalledTwice_ReturnsSameCode) {
    Process child = start_child({"exit", "7"});
    EXPECT_EQ(child.wait(), 7);
    EXPECT_EQ(child.wait(), 7);
}

TEST(ProcessTest, Start_MissingExecutable_ReportsWhichAndWhy) {
    const std::filesystem::path missing = child_path().parent_path() / "no_such_program.exe";

    auto result = Process::start({.executable = missing});

    ASSERT_FALSE(result.has_value());
    const std::string& message = result.error().message;
    EXPECT_NE(message.find("no_such_program.exe"), std::string::npos) << message;
    EXPECT_NE(message.find("Windows error 2"), std::string::npos) << message;
}

TEST(ProcessTest, Arguments_ArriveUnchanged) {
    const auto out = scratch_file("args.txt");
    const std::vector<std::string> tricky = {
        "plain",
        "with space",
        R"(quote"inside)",
        R"(C:\trailing slash\)",
        R"(back\"slash-quote)",
        "",
        "last",
    };

    std::vector<std::string> arguments = {"args", out.string()};
    arguments.insert(arguments.end(), tricky.begin(), tricky.end());
    ASSERT_EQ(start_child(arguments).wait(), 0);

    std::string expected;
    for (const std::string& argument : tricky) {
        expected += argument + "\n";
    }
    EXPECT_EQ(read_file(out), expected);
}

TEST(ProcessTest, WorkingDirectory_IsUsedByChild) {
    const auto out = scratch_file("cwd.txt");
    const auto directory = std::filesystem::canonical(::testing::TempDir());

    ASSERT_EQ(start_child({"cwd", out.string()}, directory).wait(), 0);

    EXPECT_EQ(std::filesystem::canonical(read_file(out)), directory);
}

TEST(ProcessTest, ExitCode_IsEmptyWhileRunning_ThenSet) {
    Process child = start_child({"sleep", "500"});

    // Asked straight away, the child is still asleep.
    EXPECT_FALSE(child.exit_code().has_value());

    EXPECT_EQ(child.wait(), 0);
    ASSERT_TRUE(child.exit_code().has_value());
    EXPECT_EQ(*child.exit_code(), 0);
}

TEST(ProcessTest, Move_TransfersOwnership) {
    Process first = start_child({"exit", "9"});
    Process second = std::move(first);

    EXPECT_EQ(second.wait(), 9);

    // Move assignment releases what the target held and takes over the source.
    Process third = start_child({"exit", "1"});
    third = std::move(second);
    EXPECT_EQ(third.wait(), 9);
}

TEST(ProcessTest, NewWindowConsole_RunsChild) {
    // This is the mode ciph-studio uses to open the VM stepper. A console
    // window flashes up briefly while this test runs.
    EXPECT_EQ(start_child({"exit", "5"}, {}, ConsoleMode::new_window).wait(), 5);
}
