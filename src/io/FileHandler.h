#pragma once
#include <future>
#include <optional>
#include <string>

#include "IFileDialog.h"
#include "TextEditor.h"

class FileHandler {
 public:
  explicit FileHandler(IAppFileDialog* dialogs = nullptr);
  std::future<std::optional<std::string>> LoadFileAsync(
      const std::string& filepath);
  std::future<bool> SaveFileAsync(const std::string& filepath,
                                  std::string content);

 private:
  IAppFileDialog* m_dialogs;
};
