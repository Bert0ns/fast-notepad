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

void SessionManager::SaveSessionState(const std::vector<SessionTab>& tabs) {
  auto configDir = GetConfigDirectory();
  auto path = configDir / "session.txt";

  std::error_code ec;
  for (auto& p : std::filesystem::directory_iterator(configDir, ec)) {
    if (p.is_regular_file() &&
        p.path().filename().string().find("backup_") == 0) {
      std::filesystem::remove(p.path(), ec);
    }
  }

  std::ofstream file(path);
  if (file.is_open()) {
    for (size_t i = 0; i < tabs.size(); ++i) {
      const auto& tab = tabs[i];
      file << tab.filePath << '\n';
      file << (tab.isDirty ? "1" : "0") << '\n';
      if (tab.isDirty) {
        std::string backupName = "backup_" + std::to_string(i) + ".txt";
        file << backupName << '\n';
        std::ofstream backupFile(configDir / backupName, std::ios::binary);
        if (backupFile.is_open()) {
          backupFile.write(tab.content.c_str(), tab.content.size());
        }
      } else {
        file << '\n';
      }
    }
  }
}

bool SessionManager::LoadSessionState(std::vector<SessionTab>& tabs) {
  auto configDir = GetConfigDirectory();
  auto path = configDir / "session.txt";

  std::ifstream file(path);
  if (!file.is_open()) return false;

  std::string filepath, isDirtyStr, backupName;
  while (std::getline(file, filepath)) {
    if (!filepath.empty() && filepath.back() == '\r') filepath.pop_back();

    if (!std::getline(file, isDirtyStr)) break;
    if (!isDirtyStr.empty() && isDirtyStr.back() == '\r') isDirtyStr.pop_back();

    if (!std::getline(file, backupName)) break;
    if (!backupName.empty() && backupName.back() == '\r') backupName.pop_back();

    SessionTab tab;
    tab.filePath = filepath;
    tab.isDirty = (isDirtyStr == "1");

    if (tab.isDirty && !backupName.empty()) {
      std::ifstream backupFile(configDir / backupName, std::ios::binary);
      if (backupFile.is_open()) {
        std::ostringstream ss;
        ss << backupFile.rdbuf();
        tab.content = ss.str();
      }
    }
    tabs.push_back(tab);
  }

  return !tabs.empty();
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
