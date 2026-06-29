#include "SessionManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace {
std::filesystem::path GetConfigDirectory() {
  std::filesystem::path configDir;
#ifdef _WIN32
  const char* appdata = std::getenv("APPDATA");
  if (appdata) {
    configDir = std::filesystem::path(appdata) / "FastNotepad";
  } else {
    std::error_code ec;
    configDir = std::filesystem::temp_directory_path(ec) / "FastNotepad";
  }
#else
  const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
  if (xdgConfig && *xdgConfig) {
    configDir = std::filesystem::path(xdgConfig) / "fast-notepad";
  } else {
    const char* home = std::getenv("HOME");
    if (home) {
      configDir = std::filesystem::path(home) / ".config" / "fast-notepad";
    } else {
      std::error_code ec;
      configDir = std::filesystem::temp_directory_path(ec) / "fast-notepad";
    }
  }
#endif
  std::error_code ec;
  std::filesystem::create_directories(configDir, ec);
  return configDir;
}
}


void SessionManager::SaveSessionState(const std::string& currentFilePath,
                                      const std::string& content) {
  auto path = GetConfigDirectory() / "autosave.txt";

  std::ofstream file(path, std::ios::binary);
  if (file.is_open()) {
    file << currentFilePath << '\n';
    file << content;
  }
}

bool SessionManager::LoadSessionState(std::string& currentFilePath,
                                      std::string& content) {
  auto path = GetConfigDirectory() / "autosave.txt";

  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) return false;

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

void SessionManager::SaveSettings(const AppSettings& settings) {
  auto path = GetConfigDirectory() / "settings.txt";

  std::ofstream file(path);
  if (file.is_open()) {
    file << (settings.enableMarkdown ? "1" : "0") << '\n';
    file << (settings.isDarkTheme ? "1" : "0") << '\n';
    file << settings.currentFontIndex << '\n';
  }
}

bool SessionManager::LoadSettings(AppSettings& settings) {
  auto path = GetConfigDirectory() / "settings.txt";

  std::ifstream file(path);
  if (file.is_open()) {
    std::string line;
    if (std::getline(file, line)) settings.enableMarkdown = (line == "1");
    if (std::getline(file, line)) settings.isDarkTheme = (line == "1");
    if (std::getline(file, line)) settings.currentFontIndex = std::stoi(line);
    return true;
  }
  return false;
}
