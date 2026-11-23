# ciph-lang

A learning project for getting a deeper understanding of programming languages:
a compiler (`ciph-compiler`) that turns ciph source into bytecode, a virtual
machine (`ciph-vm`) that runs it, and a node graph editor (`ciph-studio`).

## Project layout

```
source/
  shared/    ciph::shared    bytecode definitions, disassembler
  compiler/  ciph::compiler  lexer, parser, semantic analyzer, code generator
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

## The language today

```
// counts to five
let i = 0;
while (i < 5) {
    i++;
}
return i;
```

- Every value is a 16-bit integer.
- `let` declares a variable. `return` ends the program with a value.
- `while` loops. The body runs before the condition is first checked.
- Expressions use `+ - * /`, parentheses, and the comparisons `==`, `!=`, `<`
  and `>`.
- Simple statements end with `;`, including the last one in a block. Blocks
  (`while`, `fn`) take no `;` after the `}`.
- `//` starts a comment that runs to the end of the line.

- An expression takes one operator per level: `2 * 2 + 3` works, `1 + 2 + 3`
  does not yet.

Functions are in progress: `fn name() { ... }` and calls to it are parsed and
checked, but the compiler then stops with `not supported yet`, because
function bodies are not compiled. There are no parameters.

The full grammar is in [docs/grammar.md](docs/grammar.md).

## How source becomes a running program

```
source text
  -> lexer              tokens, each with a line, column and length
  -> parser             a tree (AST); each node records its tokens
  -> semantic analyzer  checks names: declared before use, not declared twice
  -> code generator     bytecode
  -> ciph-vm            runs the bytecode
```

The compiler and the VM are separate programs that pass files to each other.

Every stage reports its errors the same way, as
`<file>:<line>:<column>: error: <what>`, and a stage only runs if the one
before it found none. The parser stops at the first syntax error; the
analyzer and the code generator report everything they find.

## Building

You need CMake 3.28 or newer, Ninja, and a C++23 compiler (GCC, Clang or
MSVC). Dependencies (fmt, nlohmann/json, GoogleTest and GLFW) come from your system if CMake
can find them. Otherwise they are downloaded automatically. Dear ImGui,
imnodes and ImGuiColorTextEdit are always downloaded.

`ciph-studio` is only built on Windows for now, because it relies on
`ciph::platform`. It needs OpenGL. Pass `-DCIPH_BUILD_STUDIO=OFF` to skip it.

```sh
cmake --preset dev            # configure -> .build/dev
cmake --build --preset dev    # build     -> dist/Debug
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
- `dist/<Config>/` holds the final executables. All presets share it, so
  building with another preset replaces them.

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
