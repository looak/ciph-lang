#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <ciph/compiler/diagnostics.hpp>
#include <ciph/compiler/lexer.hpp>

namespace ciph {

class ASTBaseNode;

// The top-level container for one compilation: it owns the source text, and
// every later stage holds std::string_views into it.
//
// The text sits behind a unique_ptr so moving a CompileUnit does not move the
// characters, which keeps those views valid.
//
// A unit does not print or stop on errors. Each stage adds what it finds to
// the unit's diagnostics and says whether it succeeded; whoever drives the
// units, possibly several of them, reads the diagnostics and decides.
class CompileUnit {
public:
    explicit CompileUnit(std::string source);
    ~CompileUnit();

    CompileUnit(CompileUnit&&) noexcept;
    CompileUnit& operator=(CompileUnit&&) noexcept;

    std::string_view readSource() const { return *m_source; }

    // Turns the source into tokens. Returns false if the lexer reported
    // anything; the reason is then in readDiagnostics().
    bool lex();
    // lex() has run, whether or not it found errors. The lexer keeps that
    // state; the unit only asks it.
    bool hasLexed() const { return m_lexer->hasLexed(); }

    // Builds the AST from the tokens. Returns false, and does nothing, unless
    // lex() has succeeded. If parsing fails the reason is in readDiagnostics()
    // and astRoot() is null.
    bool parse();
    bool hasParsed() const { return m_astRoot != nullptr; }

    bool semanticAnalyze();
    bool hasAnalyzed() const { return m_semanticAnalyzed; }

    // Generates bytecode from the analyzed program. Returns false, and
    // generates nothing, unless semanticAnalyze() has passed without errors.
    bool generateCode();
    bool hasGenerated() const { return m_codeGenerated; }

    // The compiled program. Empty until generateCode() has succeeded.
    const std::vector<uint8_t>& readBytecode() const { return m_bytecode; }

    // The parsed program. Null until parse() has succeeded.
    const ASTBaseNode* astRoot() const { return m_astRoot.get(); }

    // The tokens the program was parsed from, owned by the unit's lexer. An
    // AST node's SourceRange holds indices into this list. Empty until lex().
    const std::vector<Token>& readTokens() const { return m_lexer->readTokens(); }

    // Everything the stages run so far have reported, in the order found.
    const std::vector<Diagnostic>& readDiagnostics() const { return m_diagnostics; }
    bool hasErrors() const { return !m_diagnostics.empty(); }

private:
    std::unique_ptr<const std::string> m_source;
    std::unique_ptr<Lexer> m_lexer;
    std::unique_ptr<ASTBaseNode> m_astRoot;
    std::vector<Diagnostic> m_diagnostics;
    bool m_semanticAnalyzed = false;
    std::vector<uint8_t> m_bytecode;
    bool m_codeGenerated = false;
};

} // namespace ciph
