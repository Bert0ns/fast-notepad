#pragma once
#include <imgui.h>

#include <array>
#include <string>

#include "TextEditor.h"

class FindPanel {
 public:
  void Render(TextEditor& editor, ImGuiViewport* viewport);
  void Open(TextEditor& editor, bool replaceMode = false);
  void Close();
  bool IsOpen() const { return m_show; }

 public:  // Exposed for testing
  void UpdateBufferFromSelection(TextEditor& editor);
  bool FindNextMatch(TextEditor& editor, const std::string& query, bool wrap,
                     bool& wrapped);

  int ByteIndexToColumn(const std::string& line, int byteIndex, int tabSize);
  int ColumnToByteIndex(const std::string& line, int column, int tabSize);

  void ReplaceNext(TextEditor& editor);
  void ReplaceAll(TextEditor& editor);

  bool m_show = false;
  bool m_showReplace = false;
  bool m_focusInput = false;
  bool m_focusReplace = false;
  bool m_wrapSearch = true;
  bool m_matchCase = false;
  bool m_wholeWord = false;
  bool m_findFailed = false;
  bool m_findWrapped = false;
  int m_replaceCount = -1;

  size_t FindInString(const std::string& text, const std::string& query,
                      size_t from);
  std::array<char, 256> m_findBuffer = {0};
  std::array<char, 256> m_replaceBuffer = {0};
};
