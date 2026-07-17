#include <catch2/catch_test_macros.hpp>

#include "TextEditor.h"

TEST_CASE("TextEditor Cursor Navigation Boundaries", "[TextEditor]") {
  TextEditor editor;
  editor.SetText("Line 1\nLine 2");

  SECTION("MoveDown on last line moves to end of line") {
    // Place cursor at start of the last line
    editor.SetCursorPosition(TextEditor::Coordinates(1, 0));
    editor.MoveDown();

    auto pos = editor.GetCursorPosition();
    REQUIRE(pos.mLine == 1);
    REQUIRE(pos.mColumn == 6);  // Length of "Line 2"
  }

  SECTION("MoveUp on first line moves to start of line") {
    // Place cursor at end of the first line
    editor.SetCursorPosition(TextEditor::Coordinates(0, 6));
    editor.MoveUp();

    auto pos = editor.GetCursorPosition();
    REQUIRE(pos.mLine == 0);
    REQUIRE(pos.mColumn == 0);
  }
}
