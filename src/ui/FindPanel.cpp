#include "FindPanel.h"

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
    const size_t pos = text.find(query, from);
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
    size_t pos = text.find(query, 0u);
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

  if (editor.HasSelection() && editor.GetSelectedText() == query) {
    editor.Delete();
    editor.InsertText(replaceWith);
  }

  m_findFailed = !FindNextMatch(editor, query, m_wrapSearch, m_findWrapped);
  m_replaceCount = -1;
}

void FindPanel::ReplaceAll(TextEditor& editor) {
  std::string query(m_findBuffer.data());
  if (query.empty()) return;
  std::string replaceWith(m_replaceBuffer.data());

  editor.SetCursorPosition({0, 0});
  int count = 0;
  bool wrapped = false;

  while (FindNextMatch(editor, query, false, wrapped)) {
    editor.Delete();
    editor.InsertText(replaceWith);
    count++;
  }

  m_replaceCount = count;
  m_findFailed = false;
  m_findWrapped = false;
}

void FindPanel::Render(TextEditor& editor, ImGuiViewport* viewport) {
  if (!m_show) return;

  ImGui::SetNextWindowPos(
      ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 340.0f,
             viewport->WorkPos.y + 12.0f),
      ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(320.0f, 0.0f), ImGuiCond_Always);
  ImGuiWindowFlags findFlags = ImGuiWindowFlags_NoCollapse |
                               ImGuiWindowFlags_NoSavedSettings |
                               ImGuiWindowFlags_AlwaysAutoResize;

  if (ImGui::Begin("Find", &m_show, findFlags)) {
    if (m_focusInput) {
      ImGui::SetKeyboardFocusHere();
      m_focusInput = false;
    }

    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
    bool submit = ImGui::InputText("##FindInput", m_findBuffer.data(),
                                   m_findBuffer.size(), inputFlags);
    ImGui::SameLine();
    if (ImGui::Button("Find Next")) submit = true;

    ImGui::SameLine();
    ImGui::Checkbox("Wrap", &m_wrapSearch);
    ImGui::SameLine();
    ImGui::Checkbox("Replace Mode", &m_showReplace);

    if (submit) {
      std::string query(m_findBuffer.data());
      m_findFailed = !FindNextMatch(editor, query, m_wrapSearch, m_findWrapped);
      m_replaceCount = -1;
      if (!m_findFailed) m_show = true;
    }

    if (m_showReplace) {
      if (m_focusReplace) {
        ImGui::SetKeyboardFocusHere();
        m_focusReplace = false;
      }
      ImGuiInputTextFlags repFlags = ImGuiInputTextFlags_EnterReturnsTrue;
      bool repSubmit =
          ImGui::InputText("##ReplaceInput", m_replaceBuffer.data(),
                           m_replaceBuffer.size(), repFlags);
      ImGui::SameLine();
      if (ImGui::Button("Replace")) repSubmit = true;
      ImGui::SameLine();
      if (ImGui::Button("Replace All")) {
        ReplaceAll(editor);
      }

      if (repSubmit) {
        ReplaceNext(editor);
      }
    }

    if (m_replaceCount >= 0) {
      ImGui::Text("%d occurrence(s) replaced.", m_replaceCount);
    } else if (m_findFailed) {
      ImGui::TextUnformatted("No matches found.");
    } else if (m_findWrapped) {
      ImGui::TextUnformatted("Wrapped to start.");
    }
  }
  ImGui::End();
}
