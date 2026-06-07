#pragma once
#include <string>

#include "TextEditor.h"

class FileHandler {
 public:
  void HandleDialogs(std::string& currentFilePath, TextEditor& editor,
                     bool& triggerOpen, bool& triggerSave, bool& triggerSaveAs);
  void LoadFile(const std::string& filepath, TextEditor& editor,
                std::string& currentFilePath);
  void SaveFile(const std::string& filepath, TextEditor& editor,
                std::string& currentFilePath);
};
