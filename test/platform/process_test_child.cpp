// Helper program started by the ciph::platform process tests. What it does is
// chosen by its first argument:
//
//   exit <code>            exits with <code>
//   args <file> [args...]  writes each remaining argument to <file>, one per line
//   cwd <file>             writes its working directory to <file>
//   sleep <milliseconds>   sleeps, then exits with 0
//
// Anything else exits with 100, so a test that launches it wrongly fails loudly.

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <thread>

int
main(int argc, char* argv[]) {
    if (argc < 3) {
        return 100;
    }
    const std::string_view mode = argv[1];

    if (mode == "exit") {
        return std::atoi(argv[2]);
    }
    if (mode == "args") {
        std::ofstream out(argv[2], std::ios::binary);
        for (int i = 3; i < argc; ++i) {
            out << argv[i] << '\n';
        }
        return out ? 0 : 101;
    }
    if (mode == "cwd") {
        std::ofstream out(argv[2], std::ios::binary);
        out << std::filesystem::current_path().string();
        return out ? 0 : 101;
    }
    if (mode == "sleep") {
        std::this_thread::sleep_for(std::chrono::milliseconds(std::atoi(argv[2])));
        return 0;
    }
    return 100;
}
