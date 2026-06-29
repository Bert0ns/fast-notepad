#pragma once
#include <imgui.h>

#include <array>
#include <string>

#include "TextEditor.h"

class FindPanel {
 public:
  void Render(TextEditor& editor, ImGuiViewport* viewport);
  void Open(TextEditor& editor);
  void Close();
  bool IsOpen() const { return m_show; }

 private:
  void UpdateBufferFromSelection(TextEditor& editor);
  bool FindNextMatch(TextEditor& editor, const std::string& query, bool wrap,
                     bool& wrapped);

  int ByteIndexToColumn(const std::string& line, int byteIndex, int tabSize);
  int ColumnToByteIndex(const std::string& line, int column, int tabSize);

  bool m_show = false;
  bool m_focusInput = false;
  bool m_wrapSearch = true;
  bool m_findFailed = false;
  bool m_findWrapped = false;
  std::array<char, 256> m_findBuffer = {0};
};
