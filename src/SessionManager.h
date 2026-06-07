#pragma once
#include <string>

struct AppSettings {
  bool enableMarkdown = true;
  bool isDarkTheme = true;
  int currentFontIndex = 3;
};

class SessionManager {
 public:
  static void SaveSessionState(const std::string& currentFilePath,
                               const std::string& content);
  static bool LoadSessionState(std::string& currentFilePath,
                               std::string& content);
  static void SaveSettings(const AppSettings& settings);
  static bool LoadSettings(AppSettings& settings);
};
