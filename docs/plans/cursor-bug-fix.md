# Plan: Fix Disappearing Cursor on Down Arrow

## 1. Bug Analysis & Root Cause

The user reported that pressing the Down Arrow on the last line causes the cursor to disappear.
This happens in the `ImGuiColorTextEdit` library (`TextEditor.cpp`) because:

1. **Missing Scroll Snap:** When pressing Down Arrow on the last line, `TextEditor::MoveDown()` evaluates that the new line is the same as the old line. Because `oldPos == mState.mCursorPosition`, the method skips calling `EnsureCursorVisible()`.
2. **ImGui Default Scroll:** At the exact same time, Dear ImGui processes the Down Arrow as a default keyboard navigation event, scrolling the child window down. Since `EnsureCursorVisible()` isn't called to counteract this, the window scrolls down and pushes the last line (and the cursor) out of the viewport.
3. **Missing Blink Reset:** `MoveDown()` doesn't update the cursor blink timer (`mStartTime`), so the cursor might randomly be in its "invisible" blink phase when the user presses an arrow key, adding to the unresponsiveness.

## 2. Proposed Changes

We will fix this by modifying the source code directly in the local repository clone located at `../ImGuiColorTextEdit`.

- [ ] **Modify `TextEditor.cpp` in `../ImGuiColorTextEdit`:**
  - Update `TextEditor::MoveDown()` and `TextEditor::MoveUp()` to unconditionally call `EnsureCursorVisible()` after a navigation keypress. This prevents ImGui from scrolling the cursor out of view.
  - When pressing Down on the last line, automatically move the cursor to the end of the line (matching standard editor behavior). Similarly, move to the start of the line for UpArrow on the first line.
  - Reset `mStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()` in all arrow key navigations (`MoveUp`, `MoveDown`, `MoveLeft`, `MoveRight`) to keep the cursor solidly visible while navigating.

- [ ] **Testing:**
  - Temporarily point `FastNotepad`'s `CMakeLists.txt` to the local `../ImGuiColorTextEdit` folder (using `FetchContent_Declare` with `SOURCE_DIR`, or similar) so we can compile and verify the fix locally.
  - Create a Catch2 test `tests/test_text_editor_nav.cpp` in `FastNotepad` to verify `MoveDown` and `MoveUp` boundary behaviors programmatically.

- [ ] **Commit Fixes:**
  - Once verified, I can format the code and wait for approval to commit the changes to the `ImGuiColorTextEdit` repository.
