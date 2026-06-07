#include <imgui.h>

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <string>

#include "TextEditor.h"

// Dirty trick to access private members for unit testing
#define private public
#include "FindPanel.h"
#undef private

TEST_CASE("FindPanel Logic", "[FindPanel]") {
  // TextEditor requires ImGui context for Palette initialization
  ImGui::CreateContext();

  FindPanel panel;
  TextEditor editor;

  SECTION("Open and Close state") {
    REQUIRE(panel.IsOpen() == false);
    panel.Open(editor);
    REQUIRE(panel.IsOpen() == true);
    panel.Close();
    REQUIRE(panel.IsOpen() == false);
  }

  SECTION("UTF-8 Length Calculation") {
    REQUIRE(panel.Utf8CharLength('a') == 1);
    REQUIRE(panel.Utf8CharLength(0xC3) == 2);
    REQUIRE(panel.Utf8CharLength(0xE0) == 3);
    REQUIRE(panel.Utf8CharLength(0xF0) == 4);
  }

  SECTION("Column and Byte conversion") {
    std::string line = "a\tb";
    // Tab size = 4
    // 'a' is byte 0, col 0
    // '\t' is byte 1, col 1
    // 'b' is byte 2, col 4
    REQUIRE(panel.ByteIndexToColumn(line, 0, 4) == 0);
    REQUIRE(panel.ByteIndexToColumn(line, 1, 4) == 1);
    REQUIRE(panel.ByteIndexToColumn(line, 2, 4) == 4);
    REQUIRE(panel.ByteIndexToColumn(line, 3, 4) == 5);

    REQUIRE(panel.ColumnToByteIndex(line, 0, 4) == 0);
    REQUIRE(panel.ColumnToByteIndex(line, 1, 4) == 1);
    REQUIRE(panel.ColumnToByteIndex(line, 4, 4) == 2);
    REQUIRE(panel.ColumnToByteIndex(line, 5, 4) == 3);
  }

  SECTION("UpdateBufferFromSelection") {
    editor.SetText("Line 1\nLine 2\nLine 3");
    editor.SetSelection({1, 0}, {1, 4});  // Select "Line" on line 2

    panel.UpdateBufferFromSelection(editor);
    std::string buffer(panel.m_findBuffer.data());
    REQUIRE(buffer == "Line");
  }

  SECTION("FindNextMatch without wrap") {
    editor.SetText("The quick brown fox\njumps over the lazy dog");
    bool wrapped = false;

    editor.SetCursorPosition({0, 0});
    bool found = panel.FindNextMatch(editor, "fox", false, wrapped);
    REQUIRE(found == true);
    REQUIRE(wrapped == false);
    REQUIRE(editor.GetCursorPosition().mLine == 0);
    REQUIRE(editor.GetCursorPosition().mColumn == 19);

    // Not found
    found = panel.FindNextMatch(editor, "cat", false, wrapped);
    REQUIRE(found == false);
    REQUIRE(wrapped == false);
  }

  SECTION("FindNextMatch with wrap") {
    editor.SetText("one two three\nfour five six");
    bool wrapped = false;

    editor.SetCursorPosition({1, 13});  // End of document
    bool found = panel.FindNextMatch(editor, "two", true, wrapped);

    REQUIRE(found == true);
    REQUIRE(wrapped == true);
    REQUIRE(editor.GetCursorPosition().mLine == 0);
    REQUIRE(editor.GetCursorPosition().mColumn == 7);
  }

  SECTION("Render Panel") {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800, 600);

    // ImGui requires the font atlas to be built before the first NewFrame()
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    ImGui::NewFrame();
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    panel.Open(editor);
    panel.Render(editor, viewport);

    // Render sets focus input flag to false
    REQUIRE(panel.m_focusInput == false);

    ImGui::EndFrame();
  }

  ImGui::DestroyContext();
}
