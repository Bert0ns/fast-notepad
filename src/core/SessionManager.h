#pragma once
#include <string>
#include <vector>

struct AppSettings {
  bool isDarkTheme = true;
  int currentFontIndex = 3;
};

struct SessionTab {
  std::string filePath;
  bool isDirty = false;
  std::string content;
};

class SessionManager {
 public:
  static void SaveSessionState(const std::vector<SessionTab>& tabs);
  static bool LoadSessionState(std::vector<SessionTab>& tabs);
  static void SaveSettings(const AppSettings& settings);
  static bool LoadSettings(AppSettings& settings);
};
