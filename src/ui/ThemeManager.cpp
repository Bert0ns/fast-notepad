#include "ThemeManager.h"

ThemeManager::ThemeManager() {
  m_markdownDef = GetMarkdownDefinition();
  m_cssDef = GetCssDefinition();
  m_jsDef = GetJsDefinition();
  m_htmlDef = GetHtmlDefinition();

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

TextEditor::LanguageDefinition ThemeManager::GetHtmlDefinition() {
  TextEditor::LanguageDefinition lang;
  lang.mName = "HTML";

  // HTML tags
  lang.mTokenRegexStrings.push_back(
      std::make_pair("</?[a-zA-Z0-9_\\-]+", TextEditor::PaletteIndex::Keyword));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("<![a-zA-Z0-9_\\-]+", TextEditor::PaletteIndex::Keyword));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("/?>", TextEditor::PaletteIndex::Keyword));

  // HTML attributes
  lang.mTokenRegexStrings.push_back(std::make_pair(
      "[a-zA-Z0-9_\\-]+=", TextEditor::PaletteIndex::KnownIdentifier));

  // Strings
  lang.mTokenRegexStrings.push_back(
      std::make_pair("\"[^\"]*\"", TextEditor::PaletteIndex::String));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("'[^']*'", TextEditor::PaletteIndex::String));

  // HTML Entities & Numbers
  lang.mTokenRegexStrings.push_back(
      std::make_pair("&[a-zA-Z0-9]+;", TextEditor::PaletteIndex::Number));
  lang.mTokenRegexStrings.push_back(std::make_pair(
      "[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?",
      TextEditor::PaletteIndex::Number));

  lang.mCommentStart = "<!--";
  lang.mCommentEnd = "-->";
  lang.mSingleLineComment = "";
  lang.mPreprocChar = 0;
  lang.mAutoIndentation = true;
  lang.mCaseSensitive = false;

  // Link SubLanguages
  TextEditor::LanguageDefinition::SubLanguage cssSubLang;
  cssSubLang.mStartRegex = "<style.*?>";
  cssSubLang.mEndString = "</style>";
  cssSubLang.mDefinition = &m_cssDef;
  lang.mSubLanguages.push_back(cssSubLang);

  TextEditor::LanguageDefinition::SubLanguage jsSubLang;
  jsSubLang.mStartRegex = "<script.*?>";
  jsSubLang.mEndString = "</script>";
  jsSubLang.mDefinition = &m_jsDef;
  lang.mSubLanguages.push_back(jsSubLang);

  return lang;
}

TextEditor::LanguageDefinition ThemeManager::GetCssDefinition() {
  TextEditor::LanguageDefinition lang;
  lang.mName = "CSS";
  lang.mTokenRegexStrings.push_back(std::make_pair(
      "\\.[a-zA-Z0-9_\\-]+", TextEditor::PaletteIndex::Preprocessor));
  lang.mTokenRegexStrings.push_back(std::make_pair(
      "#[a-zA-Z0-9_\\-]+", TextEditor::PaletteIndex::Preprocessor));
  lang.mTokenRegexStrings.push_back(std::make_pair(
      "[a-zA-Z\\-]+[ \\t]*:", TextEditor::PaletteIndex::KnownIdentifier));
  lang.mTokenRegexStrings.push_back(std::make_pair(
      "[a-zA-Z_][a-zA-Z0-9_\\-]*", TextEditor::PaletteIndex::Identifier));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("\"[^\"]*\"", TextEditor::PaletteIndex::String));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("'[^']*'", TextEditor::PaletteIndex::String));
  lang.mTokenRegexStrings.push_back(std::make_pair(
      "[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?",
      TextEditor::PaletteIndex::Number));
  lang.mTokenRegexStrings.push_back(std::make_pair(
      "[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]",
      TextEditor::PaletteIndex::Punctuation));

  const char* const keywords[] = {
      "px",       "em",     "rem",       "vh",       "vw",      "rgba",
      "rgb",      "url",    "important", "calc",     "body",    "div",
      "html",     "span",   "a",         "p",        "h1",      "h2",
      "h3",       "h4",     "h5",        "h6",       "ul",      "li",
      "ol",       "table",  "tr",        "td",       "th",      "form",
      "input",    "button", "header",    "footer",   "nav",     "section",
      "article",  "main",   "aside",     "block",    "inline",  "inline-block",
      "flex",     "grid",   "none",      "hidden",   "visible", "absolute",
      "relative", "fixed",  "sticky",    "static",   "solid",   "dashed",
      "dotted",   "bold",   "italic",    "underline"};
  for (auto& k : keywords) lang.mKeywords.insert(k);

  lang.mCommentStart = "/*";
  lang.mCommentEnd = "*/";
  lang.mSingleLineComment = "";
  lang.mPreprocChar = 0;
  lang.mAutoIndentation = true;
  lang.mCaseSensitive = false;
  return lang;
}

TextEditor::LanguageDefinition ThemeManager::GetJsDefinition() {
  TextEditor::LanguageDefinition lang;
  lang.mName = "JS";
  lang.mTokenRegexStrings.push_back(std::make_pair(
      "[a-zA-Z_][a-zA-Z0-9_]*", TextEditor::PaletteIndex::Identifier));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("\"[^\"]*\"", TextEditor::PaletteIndex::String));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("'[^']*'", TextEditor::PaletteIndex::String));
  lang.mTokenRegexStrings.push_back(
      std::make_pair("`[^`]*`", TextEditor::PaletteIndex::String));
  lang.mTokenRegexStrings.push_back(std::make_pair(
      "[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?",
      TextEditor::PaletteIndex::Number));
  lang.mTokenRegexStrings.push_back(std::make_pair(
      "[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]",
      TextEditor::PaletteIndex::Punctuation));

  const char* const keywords[] = {
      "function", "var",   "let",         "const",     "if",         "else",
      "for",      "while", "return",      "class",     "import",     "export",
      "true",     "false", "null",        "undefined", "new",        "this",
      "try",      "catch", "throw",       "finally",   "break",      "continue",
      "switch",   "case",  "default",     "typeof",    "instanceof", "async",
      "await",    "yield", "delete",      "void",      "in",         "of",
      "extends",  "super", "constructor", "get",       "set"};
  for (auto& k : keywords) lang.mKeywords.insert(k);

  const char* const identifiers[] = {
      "document", "window", "console", "Math",    "JSON",  "Promise", "String",
      "Array",    "Object", "Number",  "Boolean", "Error", "Map",     "Set"};
  for (auto& k : identifiers) {
    TextEditor::Identifier id;
    id.mDeclaration = "Built-in Object";
    lang.mIdentifiers.insert(std::make_pair(std::string(k), id));
  }

  lang.mCommentStart = "/*";
  lang.mCommentEnd = "*/";
  lang.mSingleLineComment = "//";
  lang.mPreprocChar = 0;
  lang.mAutoIndentation = true;
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
        0x80ebce87;  // Light sky blue highlighter
    editor.SetPalette(palette);
    m_clearColor = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
  }
}

void ThemeManager::ApplyLanguage(AppState::Language lang, TextEditor& editor) {
  switch (lang) {
    case AppState::Language::None:
      editor.SetLanguageDefinition(m_plainTextDef);
      break;
    case AppState::Language::Markdown:
      editor.SetLanguageDefinition(m_markdownDef);
      break;
    case AppState::Language::Cpp:
      editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
      break;
    case AppState::Language::Json:
      // Note: ImGuiColorTextEdit does not have a built-in JSON parser, C++ is
      // typically close enough But let's check if it has C!
      editor.SetLanguageDefinition(TextEditor::LanguageDefinition::C());
      break;
    case AppState::Language::Html:
      editor.SetLanguageDefinition(m_htmlDef);
      break;
    case AppState::Language::GLSL:
      editor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
      break;
    case AppState::Language::Lua:
      editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
      break;
  }
}
