#pragma once

#include <string>
#include <string_view>

#include <TextEditor.h>

namespace ciph::studio {

// Syntax highlighting for ciph source. Keywords come from the compiler's lexer
// so the two cannot drift apart.
const TextEditor::Language* ciph_language();

// Docked panel holding a code editor for the body of one node.
class SourcePanel {
public:
    SourcePanel();

    // Replaces the editor contents. `title` names what is being edited.
    void show(std::string_view title, std::string_view source);

    // Shows `message` instead of an editor (nothing selected, native node, ...).
    void show_message(std::string_view message);

    [[nodiscard]] std::string text() const { return m_editor.GetText(); }
    [[nodiscard]] bool has_source() const { return m_has_source; }

    void draw();

private:
    TextEditor m_editor;
    std::string m_title;
    std::string m_message;
    bool m_has_source = false;
};

} // namespace ciph::studio
