#pragma once
#include <imgui.h>

#include "AppState.h"
#include "TextEditor.h"

class ThemeManager {
 public:
  ThemeManager();
  void ApplyTheme(bool isDarkTheme, TextEditor& editor);
  void ApplyLanguage(AppState::Language lang, TextEditor& editor);

  ImVec4 GetClearColor() const { return m_clearColor; }

 private:
  void ApplyNoCommentParsing(TextEditor::LanguageDefinition& lang);
  TextEditor::LanguageDefinition GetMarkdownDefinition();
  TextEditor::LanguageDefinition GetHtmlDefinition();
  TextEditor::LanguageDefinition GetCssDefinition();
  TextEditor::LanguageDefinition GetJsDefinition();

  TextEditor::LanguageDefinition m_markdownDef;
  TextEditor::LanguageDefinition m_htmlDef;
  TextEditor::LanguageDefinition m_cssDef;
  TextEditor::LanguageDefinition m_jsDef;
  TextEditor::LanguageDefinition m_plainTextDef;
  ImVec4 m_clearColor;
};
