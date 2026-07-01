#include "ThemeManager.h"

ThemeManager::ThemeManager() {
  m_markdownDef = GetMarkdownDefinition();

  m_plainTextDef.mName = "Plain Text";
  ApplyNoCommentParsing(m_plainTextDef);
}

void ThemeManager::ApplyNoCommentParsing(TextEditor::LanguageDefinition& lang) {
  // ImGuiColorTextEdit doesn't have a built-in way to completely disable
  // comments. We use the non-printable ASCII character \x01 (Start of Heading)
  // as a hack to ensure normal user text is never accidentally parsed as a
  // comment.
  lang.mCommentStart = "\x01";
  lang.mCommentEnd = "\x01";
  lang.mSingleLineComment = "\x01";
  lang.mPreprocChar = 0;
  lang.mAutoIndentation = false;
}

TextEditor::LanguageDefinition ThemeManager::GetMarkdownDefinition() {
  TextEditor::LanguageDefinition lang;
  lang.mName = "Markdown";
  lang.mTokenRegexStrings.push_back(
      std::make_pair("`[^`]+`", TextEditor::PaletteIndex::String));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("\\*\\*[^*]+\\*\\*", TextEditor::PaletteIndex::Keyword));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("\\*[^*]+\\*", TextEditor::PaletteIndex::KnownIdentifier));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("^[ \\t]*#+", TextEditor::PaletteIndex::Preprocessor));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("^[ \\t]*>", TextEditor::PaletteIndex::Comment));
  ApplyNoCommentParsing(lang);
  lang.mCaseSensitive = true;
  return lang;
}

void ThemeManager::ApplyTheme(bool isDarkTheme, TextEditor& editor) {
  if (isDarkTheme) {
    ImGui::StyleColorsDark();
    editor.SetPalette(TextEditor::GetDarkPalette());
    m_clearColor = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
  } else {
    ImGui::StyleColorsLight();
    auto palette = TextEditor::GetLightPalette();
    palette[(int)TextEditor::PaletteIndex::Selection] =
        0x80ffb080;  // Light sky blue highlighter
    editor.SetPalette(palette);
    m_clearColor = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
  }
}

void ThemeManager::ApplyMarkdownMode(bool enableMarkdown, TextEditor& editor) {
  editor.SetLanguageDefinition(enableMarkdown ? m_markdownDef : m_plainTextDef);
}
