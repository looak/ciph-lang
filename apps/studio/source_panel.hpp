#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <TextEditor.h>

namespace ciph::studio {

// Docked panel holding a code editor for a node's ciph source.
class SourcePanel {
public:
    // `keywords` are highlighted as language keywords. The studio gets them
    // from the compiler at startup; an empty list just means no keyword
    // colouring.
    explicit SourcePanel(const std::vector<std::string>& keywords);

    // The editor keeps a pointer to m_language, so a SourcePanel must stay
    // where it is.
    SourcePanel(const SourcePanel&) = delete;
    SourcePanel& operator=(const SourcePanel&) = delete;

    void set_text(std::string_view source) { m_editor.SetText(source); }
    [[nodiscard]] std::string text() const { return m_editor.GetText(); }

    // Draws the panel with the editor, under a heading naming what is edited.
    void draw_editor(std::string_view title);

    // Draws the panel with a message in place of the editor.
    void draw_message(std::string_view message);

private:
    TextEditor::Language m_language; // declared before m_editor, which points at it
    TextEditor m_editor;
};

} // namespace ciph::studio
