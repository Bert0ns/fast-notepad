#include "FileHandler.h"

#include <fstream>
#include <sstream>

#include "portable-file-dialogs.h"

void FileHandler::LoadFile(const std::string& filepath, TextEditor& editor,
                           std::string& currentFilePath) {
  std::ifstream file(filepath);
  if (file.is_open()) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    editor.SetText(buffer.str());
    currentFilePath = filepath;
  }
}

void FileHandler::SaveFile(const std::string& filepath, TextEditor& editor,
                           std::string& currentFilePath) {
  std::ofstream file(filepath);
  if (file.is_open()) {
    file << editor.GetText();
    currentFilePath = filepath;
  }
}

void FileHandler::HandleDialogs(std::string& currentFilePath,
                                TextEditor& editor, bool& triggerOpen,
                                bool& triggerSave, bool& triggerSaveAs) {
  if (triggerOpen) {
    auto f = pfd::open_file(
        "Choose text file", ".",
        {"Text Files", "*.txt *.md *.cpp *.h", "All Files", "*"});
    if (!f.result().empty()) {
      LoadFile(f.result()[0], editor, currentFilePath);
    }
    triggerOpen = false;
  }
  if (triggerSave) {
    if (currentFilePath.empty()) {
      triggerSaveAs = true;
    } else {
      SaveFile(currentFilePath, editor, currentFilePath);
    }
    triggerSave = false;
  }
  if (triggerSaveAs) {
    auto f = pfd::save_file("Save file", ".", {"Text Files", "*.txt *.md"});
    if (!f.result().empty()) {
      SaveFile(f.result(), editor, currentFilePath);
    }
    triggerSaveAs = false;
  }
}
