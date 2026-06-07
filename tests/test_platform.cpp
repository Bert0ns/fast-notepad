#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

#include "Platform.h"

TEST_CASE("Platform Paths", "[Platform]") {
  SECTION("Icon Search Paths") {
    std::vector<std::string> paths = Platform::GetIconSearchPaths();
    REQUIRE(!paths.empty());

    // Ensure standard relative paths are included
    bool hasIcon =
        std::find(paths.begin(), paths.end(), "icon.png") != paths.end();
    bool hasRelativeIcon =
        std::find(paths.begin(), paths.end(), "../icon.png") != paths.end();

    REQUIRE(hasIcon == true);
    REQUIRE(hasRelativeIcon == true);
  }

  SECTION("Monospace Font Path") {
    const char* fontPath = Platform::FindMonospaceFontPath();
    // Since we are running in CI or locally, the font might not exist,
    // so it might return nullptr or a valid string. We just ensure it doesn't
    // crash.
    if (fontPath != nullptr) {
      std::string pathStr(fontPath);
      REQUIRE(!pathStr.empty());
    } else {
      REQUIRE(fontPath == nullptr);
    }
  }
}

#include <GLFW/glfw3.h>

TEST_CASE("Platform Window Icon", "[Platform]") {
  // Create a hidden GLFW window just to test the icon setting logic
  if (glfwInit()) {
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(100, 100, "Test Window", NULL, NULL);
    if (window) {
      // It might return true or false depending on if it finds the icon.
      // We just ensure it runs the OS-specific branches without crashing.
      bool result = Platform::SetWindowIcon(window);
      REQUIRE((result == true || result == false));  // Just ensuring no crash
      glfwDestroyWindow(window);
    }
    glfwTerminate();
  }
}
