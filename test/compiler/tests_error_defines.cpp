#include <gtest/gtest.h>

#include <fmt/core.h>

#include <ciph/compiler/error_defines.hpp>

using namespace ciph;

// Each message starts with its own code, e.g. "e0xe003: expected identifier".
// This catches a message copied from a neighbour without its number updated.
TEST(ErrorDefinesTest, EveryMessageStartsWithItsCode) {
    for (const auto& [code, message] : error_codes_map) {
        const std::string prefix = fmt::format("e0x{:x}: ", static_cast<uint16_t>(code));
        EXPECT_EQ(message.rfind(prefix, 0), 0u) << "message '" << message << "' should start with '" << prefix << "'";
    }
}
