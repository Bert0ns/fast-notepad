#pragma once
#include <string>

class SessionManager {
 public:
  static void SaveSessionState(const std::string& currentFilePath,
                               const std::string& content);
  static bool LoadSessionState(std::string& currentFilePath,
                               std::string& content);
};
