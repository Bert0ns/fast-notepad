#pragma once
#include <string>
#include <vector>

#include "FileHandler.h"
#include "FindPanel.h"
#include "TextEditor.h"
#include "ThemeManager.h"
#include "NativeFileDialog.h"
#include "WindowContext.h"

class NotepadApp {
 public:
  NotepadApp();
  ~NotepadApp();

  bool Init();
  int Run();

 private:
  void LoadFonts();
  void HandleShortcuts();
  void RenderMenuBar();
  void RenderEditor();
  void UpdateWindowTitle();
  void SelectAllText();

  WindowContext m_windowCtx;

  TextEditor m_editor;
  ThemeManager m_themeManager;
  NativeFileDialog m_nativeDialogs;
  FileHandler m_fileHandler;
  FindPanel m_findPanel;

  std::string m_currentFilePath;
  std::string m_lastFilePath = "UNINITIALIZED";

  bool m_triggerOpen = false;
  bool m_triggerSaveAs = false;
  bool m_triggerSave = false;

  bool m_enableMarkdown = true;
  bool m_isDarkTheme = true;
  bool m_focusEditorOnStart = true;

  std::vector<struct ImFont*> m_editorFonts;
  struct ImFont* m_menuFont = nullptr;
  int m_currentFontIndex = 3;  // kDefaultFontIndex
};
