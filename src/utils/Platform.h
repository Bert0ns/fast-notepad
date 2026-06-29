#pragma once
#include <string>
#include <vector>

struct GLFWwindow;

class Platform {
 public:
  static std::vector<std::string> GetIconSearchPaths();
  static bool SetWindowIcon(GLFWwindow* window);
  static const char* FindMonospaceFontPath();
};
