#pragma once
#include <imgui.h>

#include "TextEditor.h"

class ThemeManager {
 public:
  ThemeManager();
  void ApplyTheme(bool isDarkTheme, TextEditor& editor);
  void ApplyMarkdownMode(bool enableMarkdown, TextEditor& editor);

  ImVec4 GetClearColor() const { return m_clearColor; }

 private:
  void ApplyNoCommentParsing(TextEditor::LanguageDefinition& lang);
  TextEditor::LanguageDefinition GetMarkdownDefinition();

  TextEditor::LanguageDefinition m_markdownDef;
  TextEditor::LanguageDefinition m_plainTextDef;
  ImVec4 m_clearColor;
};
