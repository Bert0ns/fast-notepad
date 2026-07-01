#pragma once
#include <string>
#include <vector>

struct AppSettings {
  bool enableMarkdown = true;
  bool isDarkTheme = true;
  int currentFontIndex = 3;
};

class SessionManager {
 public:
  static void SaveSessionState(const std::vector<std::string>& filePaths);
  static bool LoadSessionState(std::vector<std::string>& filePaths);
  static void SaveSettings(const AppSettings& settings);
  static bool LoadSettings(AppSettings& settings);
};
