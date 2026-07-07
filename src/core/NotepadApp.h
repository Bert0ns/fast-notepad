#pragma once
#include <future>
#include <memory>
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

struct EditorTab {
  TextEditor editor;
  std::string currentFilePath;
  std::string lastFilePath = "UNINITIALIZED";
  bool isDirty = false;
  uint64_t tabId = 0;

  std::future<std::optional<std::string>> loadFuture;
  std::future<bool> saveFuture;
  std::string pendingFilePath;
  bool isLoading = false;
  bool isSaving = false;
  AppState::Language language = AppState::Language::None;
};

class NotepadApp {
 public:
  NotepadApp();
  ~NotepadApp();

  bool Init();
  int Run();

 private:
  void AddNewTab(const std::string& filePath = "");
  void CloseTab(int index);
  EditorTab* GetActiveTab();

  void LoadFonts();
  void RenderEditor();
  void UpdateWindowTitle();
  void SelectAllText();
  void ExecuteNew();
  void ExecuteOpen();
  void ExecuteExit();
  void ExecuteCloseTab(int index);
  void HandlePendingAction();

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

  std::vector<std::unique_ptr<EditorTab>> m_tabs;
  int m_activeTabIndex = 0;
  int m_pendingCloseTabIndex = -1;

  int m_currentFontIndex = 3;  // kDefaultFontIndex
};
