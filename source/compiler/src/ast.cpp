#include <ciph/compiler/ast.hpp>

#include <ciph/compiler/lexer.hpp>

using namespace ciph;

namespace {

Position
endOf(const Token& token) {
    Position end = token.readPosition();
    end.column += token.readLength();
    return end;
}

} // namespace

Location
ciph::locationOf(const ASTBaseNode& node, std::span<const Token> tokens) {
    const SourceRange range = node.readSourceRange();
    if (!range.hasTokens()) {
        return Location{};
    }

    uint32_t first = range.first;
    uint32_t last = range.last != SourceRange::no_token ? range.last : range.first;
    if (range.focus != SourceRange::no_token) {
        first = range.focus;
        last = range.focus;
    }
    if (first >= tokens.size() || last >= tokens.size()) {
        return Location{};
    }

    return Location{.span = {.start = tokens[first].readPosition(), .end = endOf(tokens[last])}};
}
