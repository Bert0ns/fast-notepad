#include "SessionManager.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

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
}  // namespace

void SessionManager::SaveSessionState(
    const std::vector<std::string>& filePaths) {
  auto path = GetConfigDirectory() / "session.txt";

  std::ofstream file(path);
  if (file.is_open()) {
    for (const auto& fp : filePaths) {
      if (!fp.empty()) {
        file << fp << '\n';
      }
    }
  }
}

bool SessionManager::LoadSessionState(std::vector<std::string>& filePaths) {
  auto path = GetConfigDirectory() / "session.txt";

  std::ifstream file(path);
  if (!file.is_open()) return false;

  std::string filepath;
  while (std::getline(file, filepath)) {
    if (!filepath.empty() && filepath.back() == '\r') {
      filepath.pop_back();
    }
    if (!filepath.empty()) {
      filePaths.push_back(filepath);
    }
  }

  return !filePaths.empty();
}

void SessionManager::SaveSettings(const AppSettings& settings) {
  auto path = GetConfigDirectory() / "settings.txt";

  std::ofstream file(path);
  if (file.is_open()) {
    file << (settings.isDarkTheme ? "1" : "0") << '\n';
    file << settings.currentFontIndex << '\n';
  }
}

bool SessionManager::LoadSettings(AppSettings& settings) {
  auto path = GetConfigDirectory() / "settings.txt";

  std::ifstream file(path);
  if (file.is_open()) {
    std::string line;
    if (std::getline(file, line)) settings.isDarkTheme = (line == "1");
    if (std::getline(file, line)) settings.currentFontIndex = std::stoi(line);
    return true;
  }
  return false;
}
