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
