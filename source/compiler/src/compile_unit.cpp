#include <ciph/compiler/compile_unit.hpp>

#include <memory>
#include <utility>

#include <ciph/compiler/ast.hpp>
#include <ciph/compiler/code_generator.hpp>
#include <ciph/compiler/parser.hpp>
#include <ciph/compiler/semantic_analyzer.hpp>

using namespace ciph;

CompileUnit::CompileUnit(std::string source)
    : m_source(std::make_unique<const std::string>(std::move(source)))
    , m_lexer(std::make_unique<Lexer>(*m_source)) {
}

// Defined here, where ASTBaseNode is a complete type, so the unique_ptr can
// delete it.
CompileUnit::~CompileUnit() = default;
CompileUnit::CompileUnit(CompileUnit&&) noexcept = default;
CompileUnit& CompileUnit::operator=(CompileUnit&&) noexcept = default;

bool
CompileUnit::lex() {
    if (hasLexed()) {
        return !m_lexer->hasErrors();
    }

    m_lexer->lex();
    const auto& diagnostics = m_lexer->readDiagnostics();
    m_diagnostics.insert(m_diagnostics.end(), diagnostics.begin(), diagnostics.end());
    return !m_lexer->hasErrors();
}

bool
CompileUnit::parse() {
    if (!hasLexed() || m_lexer->hasErrors()) {
        return false;
    }

    Parser parser{TokenCursor{readTokens()}};
    Expect<ASTBaseNode*> result = parser.parse();
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

    SemanticAnalyzer analyzer(static_cast<const ASTProgramNode*>(m_astRoot.get()), readTokens());
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

    CodeGenerator generator(static_cast<const ASTProgramNode*>(m_astRoot.get()), readTokens());
    auto diagnostics = generator.generateCode();
    if (!diagnostics.empty()) {
        // Keep nothing: bytecode generated with errors is not a program.
        m_diagnostics.insert(m_diagnostics.end(), std::make_move_iterator(diagnostics.begin()),
                             std::make_move_iterator(diagnostics.end()));
        return false;
    }

    // readRawBytecode() hands over a new[] copy; take ownership of it so it
    // is freed once the bytes are in m_bytecode.
    auto [bytes, size] = generator.readRawBytecode();
    const std::unique_ptr<uint8_t[]> owned(bytes);
    m_bytecode.assign(bytes, bytes + size);

    m_codeGenerated = true;
    return true;
}
