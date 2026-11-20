#include "source_panel.hpp"

#include <imgui.h>

#include <ciph/compiler/lexar_defines.hpp>

namespace ciph::studio {

namespace {

bool
is_identifier_start(ImWchar c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool
is_digit(ImWchar c) {
    return c >= '0' && c <= '9';
}

TextEditor::Iterator
get_identifier(TextEditor::Iterator start, TextEditor::Iterator end) {
    if (start == end || !is_identifier_start(*start)) {
        return start;
    }
    auto it = start;
    while (it != end && (is_identifier_start(*it) || is_digit(*it))) {
        ++it;
    }
    return it;
}

TextEditor::Iterator
get_number(TextEditor::Iterator start, TextEditor::Iterator end) {
    auto it = start;
    while (it != end && is_digit(*it)) {
        ++it;
    }
    return it;
}

bool
is_punctuation(ImWchar c) {
    switch (c) {
    case '+': case '-': case '*': case '/': case '=': case '!':
    case '<': case '>': case '(': case ')': case '{': case '}':
    case ',': case ';': case ':':
        return true;
    default:
        return false;
    }
}

} // namespace

const TextEditor::Language*
ciph_language() {
    static const TextEditor::Language language = [] {
        TextEditor::Language lang;
        lang.name = "ciph";
        for (const auto& [keyword, token] : s_keywords) {
            (void)token;
            lang.keywords.insert(keyword);
        }
        lang.isPunctuation = is_punctuation;
        lang.getIdentifier = get_identifier;
        lang.getNumber = get_number;
        return lang;
    }();
    return &language;
}

SourcePanel::SourcePanel() {
    m_editor.SetLanguage(ciph_language());
    m_editor.SetTabSize(4);
    m_editor.SetInsertSpacesOnTabs(true);
    show_message("Select a node to see its source.");
}

void
SourcePanel::show(std::string_view title, std::string_view source) {
    m_title = title;
    m_editor.SetText(source);
    m_has_source = true;
}

void
SourcePanel::show_message(std::string_view message) {
    m_message = message;
    m_has_source = false;
}

void
SourcePanel::draw() {
    ImGui::Begin("Node source");
    if (m_has_source) {
        ImGui::TextUnformatted(m_title.c_str());
        ImGui::Separator();
        m_editor.Render("##source");
    } else {
        ImGui::TextDisabled("%s", m_message.c_str());
    }
    ImGui::End();
}

} // namespace ciph::studio
