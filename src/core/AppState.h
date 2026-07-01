#pragma once
#include <string>

struct AppState {
  int forceSelectTab = -1;
  bool triggerOpen = false;
  bool triggerSave = false;
  bool triggerSaveAs = false;
  bool triggerNew = false;
  bool triggerExit = false;

  enum class PendingAction { None, Exit, CloseTab };
  PendingAction pendingAction = PendingAction::None;
  bool showUnsavedChangesModal = false;

  enum class Language { None, Markdown, Cpp, Json, GLSL, Lua };
  bool isDarkTheme = true;
  bool focusEditorOnStart = true;
  bool showErrorPopup = false;
  std::string errorMessage = "";

  int currentFontIndex = 3;
};
