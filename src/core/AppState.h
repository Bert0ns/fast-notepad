#pragma once
#include <string>

struct AppState {
  std::string currentFilePath;
  std::string lastFilePath = "UNINITIALIZED";

  bool triggerOpen = false;
  bool triggerSave = false;
  bool triggerSaveAs = false;

  bool enableMarkdown = true;
  bool isDarkTheme = true;
  bool focusEditorOnStart = true;
  bool showErrorPopup = false;
  std::string errorMessage = "";

  int currentFontIndex = 3;
};
