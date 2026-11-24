#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <ciph/compiler/diagnostics.hpp>
#include <ciph/compiler/lexer.hpp>

namespace ciph {

class ASTBaseNode;
class ASTCallNode;
class ASTFunctionNode;
class ASTIdentifierNode;
class ASTLetNode;
class ASTProgramNode;
class ASTScopeNode;

// Checks a parsed program for errors the grammar cannot express: names used
// without a declaration, names declared twice, and (once `mut` is parsed)
// writes to immutable variables.
//
// Runs between the parser and the code generator. It only reads the tree.
//
// Scoping rules:
// - A variable is visible from the end of its `let` statement to the end of
//   the block it was declared in. `let x = x;` is an error.
// - A name cannot be declared again while it is visible, not even in a
//   nested block (no shadowing). Sibling blocks may reuse a name.
// - Functions live in their own namespace, separate from variables. A
//   function is visible everywhere in the block that defines it, also before
//   its definition, so calls may come first.
// - A function body cannot see the variables around it, only functions.
//   There are no parameters or closures yet.
class SemanticAnalyzer {
public:
    // `tokens` is the parser's token list (Parser::readTokens()). Node source
    // ranges index into it; errors use it to point at the offending name.
    SemanticAnalyzer(const ASTProgramNode* program, const std::vector<Token>& tokens)
        : m_program(program)
        , m_tokens(tokens) {}
    ~SemanticAnalyzer() = default;

    // Walks the whole program and returns every error found, in source order.
    // An empty result means the program passed.
    std::vector<Diagnostic> analyze();

private:
    // One open block: the names declared in it so far, each mapped to the
    // node that declared it.
    using VariableScope = std::unordered_map<std::string_view, const ASTLetNode*>;
    using FunctionScope = std::unordered_map<std::string_view, const ASTFunctionNode*>;

    // Opens a block for `scope`'s statements, checks them, and closes it.
    void analyzeScope(const ASTScopeNode* scope);
    // Records the functions defined directly in `scope`, before any of its
    // statements are checked, so calls can come before definitions.
    void collectFunctions(const ASTScopeNode* scope);
    void analyzeStatement(const ASTBaseNode* statement);
    void analyzeFunction(const ASTFunctionNode* function);
    void analyzeExpression(const ASTBaseNode* expression);

    void declareVariable(const ASTLetNode* let);
    void checkVariableUse(const ASTIdentifierNode* identifier);
    void checkCall(const ASTCallNode* call);

    // Searches the open blocks, innermost first. nullptr if not found.
    [[nodiscard]] const ASTLetNode* findVariable(const std::string_view name) const;
    [[nodiscard]] const ASTFunctionNode* findFunction(std::string_view name) const;

    // Where `node`'s focus token starts (SourceRange::focusToken(): a let's or
    // fn's name, otherwise the first token). Nodes without a range (built by
    // hand, not parsed) give a zero position.
    [[nodiscard]] Position positionOf(const ASTBaseNode* node) const;

    void report(ErrorCode code, Position position, std::string message);

    const ASTProgramNode* m_program;
    const std::vector<Token>& m_tokens;
    std::vector<Diagnostic> m_errors;

    // The open blocks, outermost first. Entering a block pushes, leaving pops.
    std::vector<VariableScope> m_variableScopes;
    std::vector<FunctionScope> m_functionScopes;
};

} // namespace ciph
