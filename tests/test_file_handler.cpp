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

  SECTION("HandleDialogs fast-path (Save without dialog)") {
    bool tOpen = false;
    bool tSave = true;
    bool tSaveAs = false;

    // Change text
    editor.SetText("Modified text via HandleDialogs\n");
    handler.HandleDialogs(currentFile, editor, tOpen, tSave, tSaveAs);

    REQUIRE(tSave == false);
    REQUIRE(tSaveAs ==
            false);  // Didn't trigger Save As because currentFile was set

    std::ifstream in2(testPath);
    std::stringstream buffer2;
    buffer2 << in2.rdbuf();
    REQUIRE(buffer2.str() == "Modified text via HandleDialogs\n\n");
  }

  SECTION("HandleDialogs with MockFileDialog") {
    class MockFileDialog : public IFileDialog {
    public:
      std::string openResult;
      std::string saveResult;
      std::string OpenFile() override { return openResult; }
      std::string SaveFile() override { return saveResult; }
    };
    
    MockFileDialog mockDialog;
    FileHandler mockHandler(&mockDialog);
    
    // Test Open
    bool tOpen = true, tSave = false, tSaveAs = false;
    mockDialog.openResult = testPath;
    
    // Reset editor
    editor.SetText("");
    mockHandler.HandleDialogs(currentFile, editor, tOpen, tSave, tSaveAs);
    
    REQUIRE(tOpen == false);
    REQUIRE(editor.GetText() == "New Line 1\nNew Line 2\n\n\n");
    REQUIRE(currentFile == testPath);
  }

  // Cleanup
  std::filesystem::remove(testPath);
  ImGui::DestroyContext();
}
