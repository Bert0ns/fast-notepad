#include "SessionManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>

void SessionManager::SaveSessionState(const std::string& currentFilePath,
                                      const std::string& content) {
  std::error_code ec;
  auto path =
      std::filesystem::temp_directory_path(ec) / "fast-notepad-autosave.txt";
  if (ec) path = "fast-notepad-autosave.txt";

  std::ofstream file(path, std::ios::binary);
  if (file.is_open()) {
    file << currentFilePath << '\n';
    file << content;
  }
}

bool SessionManager::LoadSessionState(std::string& currentFilePath,
                                      std::string& content) {
  std::error_code ec;
  auto path =
      std::filesystem::temp_directory_path(ec) / "fast-notepad-autosave.txt";
  if (ec) path = "fast-notepad-autosave.txt";

  std::ifstream file(path, std::ios::binary);
  if (file.is_open()) {
    std::string filepath;
    std::getline(file, filepath);
    if (!filepath.empty() && filepath.back() == '\r') {
      filepath.pop_back();
    }
    currentFilePath = filepath;

    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();

    bool isEmpty = true;
    for (char c : content) {
      if (!std::isspace(static_cast<unsigned char>(c))) {
        isEmpty = false;
        break;
      }
    }

    if (currentFilePath.empty() && isEmpty) {
      return false;
    }

    return true;
  }
  return false;
}
