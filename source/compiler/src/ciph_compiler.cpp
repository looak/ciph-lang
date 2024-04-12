#include "lexar.hpp"
#include "shared_defines.hpp"
#include <fmt/core.h>

int
main()
{ 
  fmt::print("Hello, world!\n{}", libName());  
  return 0;
}