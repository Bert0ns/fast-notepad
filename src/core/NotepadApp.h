#pragma once
#include <future>
#include <optional>
#include <string>
#include <vector>

#include "AppState.h"
#include "FileHandler.h"
#include "FindPanel.h"
#include "MenuBar.h"
#include "NativeFileDialog.h"
#include "ShortcutManager.h"
#include "TextEditor.h"
#include "ThemeManager.h"
#include "WindowContext.h"

class NotepadApp {
 public:
  NotepadApp();
  ~NotepadApp();

  bool Init();
  int Run();

 private:
  void LoadFonts();
  void RenderEditor();
  void UpdateWindowTitle();
  void SelectAllText();

  WindowContext m_windowCtx;

  TextEditor m_editor;
  ThemeManager m_themeManager;
  NativeFileDialog m_nativeDialogs;
  FileHandler m_fileHandler;
  FindPanel m_findPanel;

  MenuBar m_menuBar;
  ShortcutManager m_shortcutManager;

  AppState m_state;

  std::vector<struct ImFont*> m_editorFonts;
  struct ImFont* m_menuFont = nullptr;

  std::future<std::optional<std::string>> m_loadFuture;
  std::future<bool> m_saveFuture;
  std::string m_pendingFilePath;
  bool m_isLoading = false;
  bool m_isSaving = false;
  int m_currentFontIndex = 3;  // kDefaultFontIndex
};
