# ciph-lang

A learning project for getting a deeper understanding of programming languages:
a compiler (`ciph-compiler`) that turns ciph source into bytecode, a virtual
machine (`ciph-vm`) that runs it, and a node graph editor (`ciph-studio`).

## Project layout

```
source/
  shared/    ciph::shared    bytecode definitions, disassembler
  compiler/  ciph::compiler  lexer, parser, code generator
  vm/        ciph::vm        instructions, processing unit
  platform/  ciph::platform  OS specific code: processes, executable path (Windows only)
apps/
  compiler/  ciph-compiler   executable
  vm/        ciph-vm         executable
  studio/    ciph-studio     node graph editor (Dear ImGui + imnodes)
test/        GoogleTest suites (compiler_tests, vm_tests, platform_tests)
cmake/       project CMake modules
```

Each library keeps its public headers in `include/ciph/<lib>/`. Include them as
`#include <ciph/compiler/parser.hpp>`.

## Building

You need CMake 3.28 or newer, Ninja, and a C++20 compiler (GCC, Clang or
MSVC). Dependencies (fmt, nlohmann/json, GoogleTest and GLFW) come from your system if CMake
can find them. Otherwise they are downloaded automatically. Dear ImGui,
imnodes and ImGuiColorTextEdit are always downloaded.

`ciph-studio` is only built on Windows for now, because it relies on
`ciph::platform`. It needs OpenGL. Pass `-DCIPH_BUILD_STUDIO=OFF` to skip it.

```sh
cmake --preset dev            # configure -> .build/dev
cmake --build --preset dev    # build     -> dist/dev/Debug
ctest --preset dev            # run the tests
```

Or do all three in one go with `cmake --workflow --preset dev`.

| Configure preset | Compiler |
|---|---|
| `dev` | whatever CMake finds (`CXX`, `PATH`, MSVC dev shell) |
| `dev-clang` | `clang++` |
| `dev-gcc` | `g++` |
| `dev-msvc` | `cl` (Windows) |
| `ci` | default compiler, Release (used by GitHub Actions) |

On Windows, run MSVC builds from a *Developer PowerShell for VS*. Alternatively,
open the folder in Visual Studio or VS Code (CMake Tools), which set up the
environment for you.

## Using the tools

```sh
ciph-compiler examples/while.ciph -o while.bin   # also writes while.bin.lst
ciph-vm while.bin                                # step with Enter, "exit" to stop
ciph-studio                                      # edit, Compile, Run
```

`ciph-studio` starts `ciph-compiler` and `ciph-vm` as separate programs and
expects them next to its own executable, which is where the build puts them.

### Output directories

- `.build/<preset>/` holds generated and intermediate files, including test
  binaries.
- `dist/<preset>/<Config>/` holds the final executables.

Both directories are git-ignored and safe to delete.

### Options

| Option | Default | |
|---|---|---|
| `CIPH_BUILD_TESTS` | `ON` when top-level | build the GoogleTest suites |
| `CIPH_BUILD_STUDIO` | `ON` when top-level on Windows | build `ciph-studio` |
| `CIPH_WARNINGS_AS_ERRORS` | `OFF` | `-Werror` / `/WX` |

### Installing

```sh
cmake --install .build/dev --config Release --prefix <dir>
```
