#include <imgui.h>

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

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

  std::string currentFile = "";

  // Test LoadFile
  handler.LoadFile(testPath, editor, currentFile);

  REQUIRE(currentFile == testPath);
  // TextEditor adds \n to empty lines etc, so let's verify it contains our
  // string or equals it
  REQUIRE(editor.GetText() == testContent + "\n");

  // Test SaveFile
  std::string newContent = "New Line 1\nNew Line 2\n";
  editor.SetText(newContent);

  handler.SaveFile(testPath, editor, currentFile);

  // Read manually
  std::ifstream in(testPath);
  std::stringstream buffer;
  buffer << in.rdbuf();

  REQUIRE(buffer.str() == newContent + "\n");
  REQUIRE(currentFile == testPath);

  // Cleanup
  std::filesystem::remove(testPath);
  ImGui::DestroyContext();
}
