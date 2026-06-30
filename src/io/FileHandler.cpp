#include "FileHandler.h"

#include <fstream>
#include <sstream>

FileHandler::FileHandler(IAppFileDialog* dialogs) : m_dialogs(dialogs) {}

bool FileHandler::LoadFile(const std::string& filepath, TextEditor& editor,
                           std::string& currentFilePath) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (file.is_open()) {
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string buffer(size, '\0');
    if (file.read(&buffer[0], size)) {
      editor.SetText(buffer);
      currentFilePath = filepath;
      return true;
    }
  }
  return false;
}

bool FileHandler::SaveFile(const std::string& filepath, TextEditor& editor,
                           std::string& currentFilePath) {
  std::ofstream file(filepath);
  if (file.is_open()) {
    file << editor.GetText();
    currentFilePath = filepath;
    return true;
  }
  return false;
}

std::future<std::optional<std::string>> FileHandler::LoadFileAsync(
    const std::string& filepath) {
  return std::async(
      std::launch::async, [filepath]() -> std::optional<std::string> {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
          std::streamsize size = file.tellg();
          file.seekg(0, std::ios::beg);
          std::string buffer(size, '\0');
          if (file.read(&buffer[0], size)) {
            return buffer;
          }
        }
        return std::nullopt;
      });
}

std::future<bool> FileHandler::SaveFileAsync(const std::string& filepath,
                                             std::string content) {
  return std::async(std::launch::async,
                    [filepath, content = std::move(content)]() {
                      std::ofstream file(filepath);
                      if (file.is_open()) {
                        file << content;
                        return true;
                      }
                      return false;
                    });
}

bool FileHandler::HandleDialogs(std::string& currentFilePath,
                                TextEditor& editor, bool& triggerOpen,
                                bool& triggerSave, bool& triggerSaveAs,
                                std::string& errorMessage) {
  bool success = true;
  if (triggerOpen) {
    if (m_dialogs) {
      std::string result = m_dialogs->OpenFile();
      if (!result.empty()) {
        if (!LoadFile(result, editor, currentFilePath)) {
          errorMessage = "Failed to load file: " + result;
          success = false;
        }
      }
    }
    triggerOpen = false;
  }
  if (triggerSave) {
    if (currentFilePath.empty()) {
      triggerSaveAs = true;
    } else {
      if (!SaveFile(currentFilePath, editor, currentFilePath)) {
        errorMessage = "Failed to save file: " + currentFilePath;
        success = false;
      }
    }
    triggerSave = false;
  }
  if (triggerSaveAs) {
    if (m_dialogs) {
      std::string result = m_dialogs->SaveFile();
      if (!result.empty()) {
        if (!SaveFile(result, editor, currentFilePath)) {
          errorMessage = "Failed to save file: " + result;
          success = false;
        }
      }
    }
    triggerSaveAs = false;
  }
  return success;
}
