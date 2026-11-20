#include <gtest/gtest.h>

#include <filesystem>

#include <ciph/platform/executable_path.hpp>

using namespace ciph::platform;

TEST(ExecutablePathTest, PointsAtThisTestProgram) {
    const std::filesystem::path path = executable_path();

    EXPECT_TRUE(path.is_absolute()) << path;
    EXPECT_TRUE(std::filesystem::is_regular_file(path)) << path;
    EXPECT_EQ(path.stem(), "platform_tests");
}
