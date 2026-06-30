# Action Plan: Resolving Code Quality Issues

This plan outlines the steps required to resolve the code quality issues identified in the Fast Notepad project analysis. It is broken down into four distinct phases, ordered by risk and complexity.

## Phase 1: Dependency Management (Low Risk, High Priority)

**Goal:** Stabilize the build environment by avoiding floating Git tags.

- **Step 1:** Review `CMakeLists.txt`.
- **Step 2:** Locate the `FetchContent_Declare` blocks for:
  - `ImGuiColorTextEdit` (currently `GIT_TAG master`)
  - `portable-file-dialogs` (currently `GIT_TAG main`)
  - `stb` (currently `GIT_TAG master`)
- **Step 3:** Find the latest stable commit hash or release tag for each of these repositories on GitHub.
- **Step 4:** Replace `master`/`main` with the exact commit hashes/tags.
- **Step 5:** Run CMake configure to verify the build completes successfully.

## Phase 2: Error Handling & User Feedback (Medium Risk, High Priority)

**Goal:** Prevent silent failures during file loading and saving.

- **Step 1:** Modify `src/io/FileHandler.h` and `src/io/FileHandler.cpp`. Change the return type of `LoadFile` and `SaveFile` from `void` to `bool` (or `std::optional<std::string>` for error messages).
- **Step 2:** Update the `HandleDialogs` method (or wherever `LoadFile`/`SaveFile` are called) to check the return values.
- **Step 3:** Add an `ErrorPopup` state to `NotepadApp`. If a file fails to load/save, set the error state.
- **Step 4:** In `NotepadApp::RenderEditor()` (or similar UI rendering loop), render an `ImGui::OpenPopup` if the error state is active, displaying a modal dialog to the user (e.g., "Failed to save file: Permission denied").

## Phase 3: Refactoring `NotepadApp.cpp` (High Risk, Medium Priority)

**Goal:** Eliminate the God Class anti-pattern by splitting `NotepadApp.cpp` into smaller, focused modules.

- **Step 1:** Create `src/ui/MenuBar.h` and `MenuBar.cpp`. Move the `RenderMenuBar()` logic from `NotepadApp` to `MenuBar`, injecting necessary dependencies (like `TextEditor`, `ThemeManager`, `FileHandler`).
- **Step 2:** Create `src/core/ShortcutManager.h` and `ShortcutManager.cpp`. Move the keyboard shortcut polling from `HandleShortcuts()` into this new class.
- **Step 3:** Update `NotepadApp::Run()` to call `m_shortcutManager.Handle()` and `m_menuBar.Render()`.
- **Step 4:** Recompile and run tests (including manual UI testing) to ensure menus and shortcuts function correctly.

## Phase 4: Async File I/O (High Risk, Low Priority)

**Goal:** Keep the UI responsive when loading or saving large files.

- **Step 1:** Refactor `FileHandler::LoadFile` and `SaveFile` to accept callbacks, or use `std::async` returning a `std::future<bool>`.
- **Step 2:** In `NotepadApp`, check the status of the `std::future` in the main loop without blocking.
- **Step 3:** While the `std::future` is not ready, display a "Loading..." or "Saving..." overlay/spinner using ImGui.
- **Step 4:** Once the future resolves, update the `TextEditor` content or file path. Ensure thread safety since `ImGui` and `TextEditor` methods must only be called from the main thread.
