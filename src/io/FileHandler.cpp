#include "FileHandler.h"

#include <fstream>
#include <sstream>

FileHandler::FileHandler(IFileDialog* dialogs) : m_dialogs(dialogs) {}

void FileHandler::LoadFile(const std::string& filepath, TextEditor& editor,
                           std::string& currentFilePath) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (file.is_open()) {
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string buffer(size, '\0');
    if (file.read(&buffer[0], size)) {
      editor.SetText(buffer);
      currentFilePath = filepath;
    }
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
    if (m_dialogs) {
      std::string result = m_dialogs->OpenFile();
      if (!result.empty()) {
        LoadFile(result, editor, currentFilePath);
      }
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
    if (m_dialogs) {
      std::string result = m_dialogs->SaveFile();
      if (!result.empty()) {
        SaveFile(result, editor, currentFilePath);
      }
    }
    triggerSaveAs = false;
  }
}
