#include <ciph/compiler/compile_unit.hpp>

#include <memory>
#include <utility>

#include <ciph/compiler/ast.hpp>
#include <ciph/compiler/code_generator.hpp>
#include <ciph/compiler/parser.hpp>
#include <ciph/compiler/semantic_analyzer.hpp>

using namespace ciph;

CompileUnit::CompileUnit(std::string source)
    : m_source(std::make_unique<const std::string>(std::move(source))) {
}

// Defined here, where ASTBaseNode is a complete type, so the unique_ptr can
// delete it.
CompileUnit::~CompileUnit() = default;
CompileUnit::CompileUnit(CompileUnit&&) noexcept = default;
CompileUnit& CompileUnit::operator=(CompileUnit&&) noexcept = default;

bool
CompileUnit::parse() {
    Parser parser(*m_source);
    ParseExpect result = parser.parse();
    m_tokens = parser.readTokens();

    if (!result) {
        m_diagnostics.push_back(std::move(result.error()));
        return false;
    }

    m_astRoot.reset(*result);
    return true;
}

bool
CompileUnit::semanticAnalyze() {
    if (!hasParsed()) {
        return false;
    }

    SemanticAnalyzer analyzer(static_cast<const ASTProgramNode*>(m_astRoot.get()), m_tokens);
    auto diagnostics = analyzer.analyze();
    m_diagnostics.insert(m_diagnostics.end(), std::make_move_iterator(diagnostics.begin()), std::make_move_iterator(diagnostics.end()));
    
    m_semanticAnalyzed = true;
    return m_semanticAnalyzed && diagnostics.empty();
}


bool
CompileUnit::generateCode() {
    if (!hasAnalyzed() || hasErrors()) {
        return false;
    }

    CodeGenerator generator(static_cast<const ASTProgramNode*>(m_astRoot.get()));
    generator.generateCode();

    // readRawBytecode() hands over a new[] copy; take ownership of it so it
    // is freed once the bytes are in m_bytecode.
    auto [bytes, size] = generator.readRawBytecode();
    const std::unique_ptr<uint8_t[]> owned(bytes);
    m_bytecode.assign(bytes, bytes + size);

    m_codeGenerated = true;
    return true;
}
