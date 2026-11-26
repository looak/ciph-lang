#include <ciph/compiler/semantic_analyzer.hpp>

#include <algorithm>
#include <utility>

#include <fmt/core.h>

#include <ciph/compiler/ast.hpp>

using namespace ciph;

std::vector<Diagnostic>
SemanticAnalyzer::analyze() {
    m_errors.clear();
    m_variableScopes.clear();
    m_functionScopes.clear();

    analyzeScope(m_program);

    // Function definitions are checked before the statements around them, so
    // the walk does not quite report in source order. Sort to restore it.
    std::stable_sort(m_errors.begin(), m_errors.end(), [](const Diagnostic& a, const Diagnostic& b) {
        if (a.position.line != b.position.line) {
            return a.position.line < b.position.line;
        }
        return a.position.column < b.position.column;
    });

    return m_errors;
}

void
SemanticAnalyzer::analyzeScope(const ASTScopeNode* scope) {
    m_variableScopes.emplace_back();
    m_functionScopes.emplace_back();

    collectFunctions(scope);
    for (const ASTBaseNode* statement : scope->readStatements()) {
        analyzeStatement(statement);
    }

    m_functionScopes.pop_back();
    m_variableScopes.pop_back();
}

void
SemanticAnalyzer::collectFunctions(const ASTScopeNode* scope) {
    FunctionScope& functions = m_functionScopes.back();

    for (const ASTBaseNode* statement : scope->readStatements()) {
        if (statement->readType() != ASTNodeType::FUNCTION) {
            continue;
        }
        const auto* function = static_cast<const ASTFunctionNode*>(statement);
        std::string_view name = function->readName();

        // A function from an enclosing block counts too: no shadowing.
        if (const ASTFunctionNode* earlier = findFunction(name)) {
            const Position first = positionOf(earlier);
            report(ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER, positionOf(function),
                    fmt::format("function '{}' is already defined at line {}, column {}", name, first.line,
                                first.column));
            continue;
        }
        functions.emplace(name, function);
    }
}

void
SemanticAnalyzer::analyzeStatement(const ASTBaseNode* statement) {
    switch (statement->readType()) {
        case ASTNodeType::LET: {
            const auto* let = static_cast<const ASTLetNode*>(statement);
            // The value is checked before the name exists, so `let x = x;`
            // reports x as undeclared.
            analyzeExpression(let->readExpression());
            declareVariable(let);
            break;
        }
        case ASTNodeType::RETURN:
            analyzeExpression(static_cast<const ASTReturnNode*>(statement)->readExpression());
            break;
        case ASTNodeType::WHILE: {
            const auto* loop = static_cast<const ASTWhileNode*>(statement);
            analyzeExpression(loop->readCondition());
            analyzeScope(loop);
            break;
        }
        case ASTNodeType::FUNCTION:
            analyzeFunction(static_cast<const ASTFunctionNode*>(statement));
            break;
        default:
            // An expression used as a statement, such as `i++;`.
            analyzeExpression(statement);
            break;
    }
}

void
SemanticAnalyzer::analyzeFunction(const ASTFunctionNode* function) {
    // A function body cannot see the variables around it (see the class
    // comment). Set the open variable blocks aside while the body is checked
    // and put them back afterwards. Function blocks stay visible.
    std::vector<VariableScope> outer = std::exchange(m_variableScopes, {});
    analyzeScope(function);
    m_variableScopes = std::move(outer);
}

void
SemanticAnalyzer::analyzeExpression(const ASTBaseNode* expression) {
    // The parser can leave an expression slot empty instead of failing.
    if (expression == nullptr) {
        return;
    }

    switch (expression->readType()) {
        case ASTNodeType::IDENTIFIER:
            checkVariableUse(static_cast<const ASTIdentifierNode*>(expression));
            break;
        case ASTNodeType::CALL_EXPRESSION:
            checkCall(static_cast<const ASTCallNode*>(expression));
            break;
        case ASTNodeType::BINARY_EXPRESSION: {
            const auto* binary = static_cast<const ASTBinaryExpressionNode*>(expression);
            analyzeExpression(binary->readLeft());
            analyzeExpression(binary->readRight());
            break;
        }
        case ASTNodeType::COMPARISON_EXPRESSION: {
            const auto* comparison = static_cast<const ASTComparisonExpressionNode*>(expression);
            analyzeExpression(comparison->readLeft());
            analyzeExpression(comparison->readRight());
            break;
        }
        case ASTNodeType::UNARY_EXPRESSION:
            analyzeExpression(static_cast<const ASTUnaryExpressionNode*>(expression)->readOperand());
            break;
        default:
            // Literals and a bare ++/-- use no names.
            break;
    }
}

void
SemanticAnalyzer::declareVariable(const ASTLetNode* let) {
    std::string_view name = let->readIdentifier();

    // Searches every open block, not only the innermost: no shadowing.
    if (const ASTLetNode* earlier = findVariable(name)) {
        const Position first = positionOf(earlier);
        report(ErrorCode::SEMANTIC_ERROR_REDECLARED_IDENTIFIER, positionOf(let),
                fmt::format("'{}' is already declared at line {}, column {}", name, first.line, first.column));
        return;
    }
    m_variableScopes.back().emplace(name, let);
}

void
SemanticAnalyzer::checkVariableUse(const ASTIdentifierNode* identifier) {
    if (findVariable(identifier->readName()) == nullptr) {
        report(ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER, positionOf(identifier),
               fmt::format("'{}' is not declared at this point", identifier->readName()));
    }
}

void
SemanticAnalyzer::checkCall(const ASTCallNode* call) {
    if (findFunction(call->readFunctionName()) == nullptr) {
        report(ErrorCode::SEMANTIC_ERROR_UNDECLARED_IDENTIFIER, positionOf(call),
               fmt::format("there is no function named '{}'", call->readFunctionName()));
    }
}

const ASTLetNode*
SemanticAnalyzer::findVariable(std::string_view name) const {
    for (auto scope = m_variableScopes.rbegin(); scope != m_variableScopes.rend(); ++scope) {
        if (auto found = scope->find(name); found != scope->end()) {
            return found->second;
        }
    }
    return nullptr;
}

const ASTFunctionNode*
SemanticAnalyzer::findFunction(std::string_view name) const {
    for (auto scope = m_functionScopes.rbegin(); scope != m_functionScopes.rend(); ++scope) {
        if (auto found = scope->find(name); found != scope->end()) {
            return found->second;
        }
    }
    return nullptr;
}

Position
SemanticAnalyzer::positionOf(const ASTBaseNode* node) const {
    const SourceRange range = node->readSourceRange();
    const uint32_t token = range.focusToken();
    if (!range.hasTokens() || token >= m_tokens.size()) {
        return Position{0, 0};
    }
    return m_tokens[token].readPosition();
}

void
SemanticAnalyzer::report(ErrorCode code, Position position, std::string message) {
    m_errors.push_back(Diagnostic{.code = code, .position = position, .additionalInfo = std::move(message)});
}
