#pragma once
#include <string>

#include "TextEditor.h"
#include "IFileDialog.h"

class FileHandler {
 public:
  explicit FileHandler(IAppFileDialog* dialogs = nullptr);
  void HandleDialogs(std::string& currentFilePath, TextEditor& editor,
                     bool& triggerOpen, bool& triggerSave, bool& triggerSaveAs);
  void LoadFile(const std::string& filepath, TextEditor& editor,
                std::string& currentFilePath);
  void SaveFile(const std::string& filepath, TextEditor& editor,
                std::string& currentFilePath);

 private:
  IAppFileDialog* m_dialogs;
};
