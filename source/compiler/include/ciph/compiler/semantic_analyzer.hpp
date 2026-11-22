#pragma once

#include <vector>

#include <ciph/compiler/parser.hpp> // ParserError, until errors get a shared type

namespace ciph {

class ASTProgramNode;

// Checks a parsed program for errors the grammar cannot express: names used
// without a declaration, names declared twice, and (once `mut` is parsed)
// writes to immutable variables.
//
// Runs between the parser and the code generator. It only reads the tree.
class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(const ASTProgramNode* program)
        : m_program(program) {}
    ~SemanticAnalyzer() = default;

    // Walks the whole program and returns every error found, in source order.
    // An empty result means the program passed.
    std::vector<ParserError> analyze();

private:
    const ASTProgramNode* m_program;
    std::vector<ParserError> m_errors;
};

} // namespace ciph
