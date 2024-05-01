#pragma once

#include <spdlog/spdlog.h>

#include "error_defines.hpp"


namespace log {

void
error(ErrorReport error) {
    spdlog::error("{}: {}", error_codes_map[error.code], error.message);
}
} // namespace log