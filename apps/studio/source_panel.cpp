#include "source_panel.hpp"

#include <imgui.h>

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

SourcePanel::SourcePanel(const std::vector<std::string>& keywords) {
    m_language.name = "ciph";
    m_language.keywords.insert(keywords.begin(), keywords.end());
    m_language.isPunctuation = is_punctuation;
    m_language.getIdentifier = get_identifier;
    m_language.getNumber = get_number;

    m_editor.SetLanguage(&m_language);
    m_editor.SetTabSize(4);
    m_editor.SetInsertSpacesOnTabs(true);
}

void
SourcePanel::draw_editor(std::string_view title) {
    ImGui::Begin("Node source");
    ImGui::TextUnformatted(title.data(), title.data() + title.size());
    ImGui::Separator();
    m_editor.Render("##source");
    ImGui::End();
}

void
SourcePanel::draw_message(std::string_view message) {
    ImGui::Begin("Node source");
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextUnformatted(message.data(), message.data() + message.size());
    ImGui::PopStyleColor();
    ImGui::End();
}

} // namespace ciph::studio
