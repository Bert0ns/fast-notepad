#pragma once
#include <future>
#include <optional>
#include <string>

#include "IFileDialog.h"
#include "TextEditor.h"

class FileHandler {
 public:
  explicit FileHandler(IAppFileDialog* dialogs = nullptr);
  bool HandleDialogs(std::string& currentFilePath, TextEditor& editor,
                     bool& triggerOpen, bool& triggerSave, bool& triggerSaveAs,
                     std::string& errorMessage);
  bool LoadFile(const std::string& filepath, TextEditor& editor,
                std::string& currentFilePath);
  bool SaveFile(const std::string& filepath, TextEditor& editor,
                std::string& currentFilePath);
  std::future<std::optional<std::string>> LoadFileAsync(
      const std::string& filepath);
  std::future<bool> SaveFileAsync(const std::string& filepath,
                                  std::string content);

 private:
  IAppFileDialog* m_dialogs;
};
