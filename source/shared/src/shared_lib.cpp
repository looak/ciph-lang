#include <string>
#define SHARED_LIB_EXPORTS // unsure about the necessity of this
#include "shared_defines.hpp"

std::string libName() {
    return "shared_lib";
}