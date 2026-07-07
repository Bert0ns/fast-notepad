#include "FindPanel.h"

#include <algorithm>
#include <cctype>

#include "Utf8Utils.h"

void FindPanel::Open(TextEditor& editor, bool replaceMode) {
  UpdateBufferFromSelection(editor);
  m_show = true;
  m_showReplace = replaceMode;
  if (replaceMode)
    m_focusReplace = true;
  else
    m_focusInput = true;
  m_replaceCount = -1;
}

void FindPanel::Close() { m_show = false; }

void FindPanel::UpdateBufferFromSelection(TextEditor& editor) {
  if (!editor.HasSelection()) return;
  std::string selected = editor.GetSelectedText();
  auto newlinePos = selected.find('\n');
  if (newlinePos != std::string::npos)
    selected = selected.substr(0, newlinePos);
  if (selected.empty()) return;
  std::snprintf(m_findBuffer.data(), m_findBuffer.size(), "%s",
                selected.c_str());
}

int FindPanel::ByteIndexToColumn(const std::string& line, int byteIndex,
                                 int tabSize) {
  int column = 0;
  int i = 0;
  const int limit = std::min<int>(byteIndex, static_cast<int>(line.size()));
  while (i < limit) {
    unsigned char c = static_cast<unsigned char>(line[i]);
    if (c == '\t') {
      column = (column / tabSize) * tabSize + tabSize;
      ++i;
    } else {
      ++column;
      i += Utf8Utils::CharLength(c);
    }
  }
  return column;
}

int FindPanel::ColumnToByteIndex(const std::string& line, int column,
                                 int tabSize) {
  int currentColumn = 0;
  int i = 0;
  while (i < static_cast<int>(line.size()) && currentColumn < column) {
    unsigned char c = static_cast<unsigned char>(line[i]);
    if (c == '\t') {
      int nextColumn = (currentColumn / tabSize) * tabSize + tabSize;
      if (nextColumn > column) break;
      currentColumn = nextColumn;
      ++i;
    } else {
      ++currentColumn;
      i += Utf8Utils::CharLength(c);
    }
  }
  return i;
}

static bool IsWordBoundary(char c) {
  return !std::isalnum(static_cast<unsigned char>(c)) && c != '_';
}

size_t FindPanel::FindInString(const std::string& text,
                               const std::string& query, size_t from) {
  if (query.empty() || text.empty() || from >= text.size())
    return std::string::npos;

  auto search_it =
      std::search(text.begin() + from, text.end(), query.begin(), query.end(),
                  [this](char ch1, char ch2) {
                    if (!m_matchCase) {
                      return std::tolower(static_cast<unsigned char>(ch1)) ==
                             std::tolower(static_cast<unsigned char>(ch2));
                    }
                    return ch1 == ch2;
                  });

  while (search_it != text.end()) {
    size_t pos = std::distance(text.begin(), search_it);

    bool match = true;
    if (m_wholeWord) {
      if (pos > 0 && !IsWordBoundary(text[pos - 1])) {
        match = false;
      }
      if (match && pos + query.size() < text.size() &&
          !IsWordBoundary(text[pos + query.size()])) {
        match = false;
      }
    }

    if (match) return pos;

    search_it =
        std::search(search_it + 1, text.end(), query.begin(), query.end(),
                    [this](char ch1, char ch2) {
                      if (!m_matchCase) {
                        return std::tolower(static_cast<unsigned char>(ch1)) ==
                               std::tolower(static_cast<unsigned char>(ch2));
                      }
                      return ch1 == ch2;
                    });
  }

  return std::string::npos;
}

bool FindPanel::FindNextMatch(TextEditor& editor, const std::string& query,
                              bool wrap, bool& wrapped) {
  wrapped = false;
  if (query.empty()) return false;
  const auto lines = editor.GetTextLines();
  if (lines.empty()) return false;

  const auto cursor = editor.GetCursorPosition();
  int startLine =
      std::min<int>(cursor.mLine, static_cast<int>(lines.size() - 1));
  int startByte =
      ColumnToByteIndex(lines[startLine], cursor.mColumn, editor.GetTabSize());

  for (int line = startLine; line < static_cast<int>(lines.size()); ++line) {
    const auto& text = lines[line];
    const size_t from =
        (line == startLine) ? static_cast<size_t>(startByte) : 0u;
    const size_t pos = FindInString(text, query, from);
    if (pos != std::string::npos) {
      int startCol =
          ByteIndexToColumn(text, static_cast<int>(pos), editor.GetTabSize());
      int endCol = ByteIndexToColumn(text, static_cast<int>(pos + query.size()),
                                     editor.GetTabSize());
      editor.SetSelection({line, startCol}, {line, endCol});
      editor.SetCursorPosition({line, endCol});
      return true;
    }
  }

  if (!wrap) return false;

  for (int line = 0; line <= startLine; ++line) {
    const auto& text = lines[line];
    const size_t limit =
        (line == startLine) ? static_cast<size_t>(startByte) : text.size();
    size_t pos = FindInString(text, query, 0u);
    if (pos != std::string::npos && pos < limit) {
      int startCol =
          ByteIndexToColumn(text, static_cast<int>(pos), editor.GetTabSize());
      int endCol = ByteIndexToColumn(text, static_cast<int>(pos + query.size()),
                                     editor.GetTabSize());
      editor.SetSelection({line, startCol}, {line, endCol});
      editor.SetCursorPosition({line, endCol});
      wrapped = true;
      return true;
    }
  }
  return false;
}

void FindPanel::ReplaceNext(TextEditor& editor) {
  std::string query(m_findBuffer.data());
  if (query.empty()) return;
  std::string replaceWith(m_replaceBuffer.data());

  if (editor.HasSelection()) {
    std::string selected = editor.GetSelectedText();
    bool match = false;
    if (m_matchCase) {
      match = (selected == query);
    } else {
      if (selected.size() == query.size()) {
        match = true;
        for (size_t i = 0; i < selected.size(); ++i) {
          if (std::tolower(static_cast<unsigned char>(selected[i])) !=
              std::tolower(static_cast<unsigned char>(query[i]))) {
            match = false;
            break;
          }
        }
      }
    }

    if (match) {
      editor.Delete();
      editor.InsertText(replaceWith);
    }
  }

  m_findFailed = !FindNextMatch(editor, query, m_wrapSearch, m_findWrapped);
  m_replaceCount = -1;
}

void FindPanel::ReplaceAll(TextEditor& editor) {
  std::string query(m_findBuffer.data());
  if (query.empty()) return;
  std::string replaceWith(m_replaceBuffer.data());

  std::string fullText = editor.GetText();
  if (fullText.empty()) return;

  if (fullText.back() == '\n') {
    fullText.pop_back();
  }

  size_t pos = 0;
  int count = 0;
  while ((pos = FindInString(fullText, query, pos)) != std::string::npos) {
    fullText.replace(pos, query.length(), replaceWith);
    pos += replaceWith.length();
    count++;
  }

  if (count > 0) {
    editor.SetText(fullText);
  }

  m_replaceCount = count;
  m_findFailed = false;
  m_findWrapped = false;
}

void FindPanel::Render(TextEditor& editor, ImGuiViewport* viewport) {
  if (!m_show) return;

  ImGui::SetNextWindowPos(
      ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 412.0f,
             viewport->WorkPos.y + 12.0f),
      ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Always);
  ImGuiWindowFlags findFlags = ImGuiWindowFlags_NoCollapse |
                               ImGuiWindowFlags_NoSavedSettings |
                               ImGuiWindowFlags_AlwaysAutoResize;

  if (ImGui::Begin("Find & Replace", &m_show, findFlags)) {
    if (m_focusInput) {
      ImGui::SetKeyboardFocusHere();
      m_focusInput = false;
    }

    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Find:   ");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(200.0f);
    bool submit = ImGui::InputText("##FindInput", m_findBuffer.data(),
                                   m_findBuffer.size(), inputFlags);
    ImGui::SameLine();
    if (ImGui::Button("Find Next")) submit = true;

    if (m_showReplace) {
      if (m_focusReplace) {
        ImGui::SetKeyboardFocusHere();
        m_focusReplace = false;
      }
      ImGuiInputTextFlags repFlags = ImGuiInputTextFlags_EnterReturnsTrue;

      ImGui::AlignTextToFramePadding();
      ImGui::Text("Replace:");
      ImGui::SameLine();

      ImGui::SetNextItemWidth(200.0f);
      bool repSubmit =
          ImGui::InputText("##ReplaceInput", m_replaceBuffer.data(),
                           m_replaceBuffer.size(), repFlags);
      ImGui::SameLine();
      if (ImGui::Button("Replace")) repSubmit = true;
      ImGui::SameLine();
      if (ImGui::Button("All")) {
        ReplaceAll(editor);
      }

      if (repSubmit) {
        ReplaceNext(editor);
      }
    }

    ImGui::Separator();
    ImGui::Checkbox("Match case", &m_matchCase);
    ImGui::SameLine();
    ImGui::Checkbox("Whole word", &m_wholeWord);

    ImGui::Checkbox("Wrap search", &m_wrapSearch);
    ImGui::SameLine();
    ImGui::Checkbox("Enable Replace Mode", &m_showReplace);

    if (submit) {
      std::string query(m_findBuffer.data());
      m_findFailed = !FindNextMatch(editor, query, m_wrapSearch, m_findWrapped);
      m_replaceCount = -1;
      if (!m_findFailed) m_show = true;
    }

    if (m_replaceCount >= 0) {
      ImGui::Text("%d occurrence(s) replaced.", m_replaceCount);
    } else if (m_findFailed) {
      ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "No matches found.");
    } else if (m_findWrapped) {
      ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Wrapped to start.");
    }
  }
  ImGui::End();
}
