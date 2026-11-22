#include <ciph/compiler/semantic_analyzer.hpp>

#include <ciph/compiler/ast.hpp>

using namespace ciph;

std::vector<ParserError>
SemanticAnalyzer::analyze() {
    m_errors.clear();

    // Not implemented yet: nothing is checked, so every program passes.

    return m_errors;
}
