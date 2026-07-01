#include <imgui.h>

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "FileHandler.h"
#include "TextEditor.h"

TEST_CASE("FileHandler Load and Save", "[FileHandler]") {
  // We must create a dummy ImGui context because TextEditor requires it in its
  // constructor (It initializes palettes which use
  // ImGui::ColorConvertFloat4ToU32, etc.)
  ImGui::CreateContext();

  FileHandler handler;
  TextEditor editor;

  std::string testPath = "dummy_test_file.txt";
  std::string testContent = "Line 1\nLine 2\nLine 3\n";

  // Create a file manually
  {
    std::ofstream out(testPath);
    out << testContent;
  }

  SECTION("LoadFileAsync") {
    // Reset file contents
    {
      std::ofstream out(testPath);
      out << testContent;
    }

    auto future = handler.LoadFileAsync(testPath);
    future.wait();
    auto result = future.get();
    REQUIRE(result.has_value());
    REQUIRE(result.value() == testContent);

    // Test failure
    auto futureFail = handler.LoadFileAsync("non_existent_file.xyz");
    futureFail.wait();
    REQUIRE(futureFail.get().has_value() == false);
  }

  SECTION("SaveFileAsync") {
    std::string asyncContent = "Async Save Test\n";
    auto future = handler.SaveFileAsync(testPath, asyncContent);
    future.wait();
    REQUIRE(future.get() == true);

    std::ifstream in3(testPath);
    std::stringstream buffer3;
    buffer3 << in3.rdbuf();
    REQUIRE(buffer3.str() == asyncContent);
  }

  // Cleanup
  std::filesystem::remove(testPath);
  ImGui::DestroyContext();
}
