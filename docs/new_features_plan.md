# Fast Notepad - New Features Plan

This document outlines a detailed, step-by-step implementation plan for the new features proposed in the initial project analysis.

## Phase 1: Unsaved Changes Protection (Critical)

**Goal:** Prevent data loss when users create a new file, open an existing file, or exit the application while having unsaved changes.
**Classes Affected:** `NotepadApp`, `AppState`, `TextEditor`

**Implementation Steps:**

1. **State Tracking:** Introduce a boolean `m_isDirty` in `AppState`. Update this by hooking into the editor state or periodically comparing `m_editor.GetUndoIndex()` with a saved undo index.
2. **Action Interception:** When `triggerOpen`, `triggerNew`, or `WindowContext::ShouldClose()` are invoked, check `m_isDirty`. If true, block the immediate action and set a flag (e.g., `m_showUnsavedChangesModal = true`).
3. **Modal UI:** Create an ImGui modal (`ImGui::BeginPopupModal("Unsaved Changes")`).
   - Buttons: "Save", "Don't Save", "Cancel".
4. **Resolution Routing:**
   - _Save_: Trigger the normal Save logic. Once resolved, proceed with the intercepted action.
   - _Don't Save_: Discard changes and proceed with the intercepted action. comes as pre-selected
   - _Cancel_: Close the modal and abort the intercepted action.

**Testing Strategy:**

- Mock the file modified state and simulate a close event in `test_shortcut_manager.cpp` to ensure the modal flag is triggered instead of an immediate closure.

---

## Phase 2: Multi-Tab Support

**Goal:** Allow users to open and edit multiple files concurrently in a tabbed interface.
**Classes Affected:** `NotepadApp`, `AppState`, `FileHandler`

**Implementation Steps:**

1. **Editor Encapsulation:** Create a new `EditorTab` struct containing a `TextEditor` instance, its associated `currentFilePath`, `loadFuture`, `saveFuture`, and `isDirty` flag.
2. **State Update:** Change `AppState` to hold a `std::vector<EditorTab>` and an `int activeTabIndex`.
3. **UI Integration:** In `NotepadApp::RenderEditor`, wrap the text editor rendering in an `ImGui::BeginTabBar` and `ImGui::BeginTabItem` loop.
4. **Routing Commands:** Ensure that menu commands (Save, Find, Undo, Font scaling) apply strictly to `m_tabs[activeTabIndex]`.
5. **Session Management:** Update `SessionManager` to serialize an array of file paths instead of a single path, restoring all previously opened tabs on startup.

**Testing Strategy:**

- Verify that `EditorTab` states do not bleed into each other.
- Test `SessionManager` serialization of multiple paths.

---

## Phase 3: Find & Replace

**Goal:** Extend the existing Find functionality to allow text replacement.
**Classes Affected:** `FindPanel`, `TextEditor`

**Implementation Steps:**

1. **UI Expansion:** Add a new `ImGui::InputText` for the "Replace With" string inside `FindPanel::Render`.
2. **Replace Logic:** Add a `ReplaceNext()` method. It should locate the currently highlighted match using the `m_editor.GetSelection()` coordinates and replace the text via `m_editor.InsertText()`.
3. **Replace All Logic:** Add a `ReplaceAll()` method. It will iteratively call `FindNextMatch` and replace the text until no matches remain, bundling these changes into a single undo step if possible.
4. **Shortcut Integration:** Map `Ctrl+H` in `ShortcutManager` to open the `FindPanel` with the replace fields expanded and focused.

**Testing Strategy:**

- Write Catch2 unit tests in `test_find_panel.cpp` mocking buffer content and asserting the final buffer state after `ReplaceNext()` and `ReplaceAll()`.

---

## Phase 4: Extended Syntax Highlighting

**Goal:** Provide users with more syntax highlighting options beyond simple text and Markdown.
**Classes Affected:** `ThemeManager`, `MenuBar`, `AppState`

**Implementation Steps:**

1. **State Update:** Change the boolean `enableMarkdown` to an enum `enum class Language { None, Markdown, Cpp, Json, GLSL, Lua }`.
2. **ThemeManager Integration:** In `ThemeManager::ApplyMarkdownMode`, rename it to `ApplyLanguage(Language lang, TextEditor& editor)` and leverage `TextEditor::LanguageDefinition` (e.g., `TextEditor::LanguageDefinition::CPlusPlus()`).
3. **UI Integration:** In `MenuBar::Render`, add a "Language" sub-menu under "View", iterating through the `Language` enum with `ImGui::MenuItem`.
4. **Auto-Detection (Optional):** When loading a file, parse the extension in `NotepadApp` (e.g., `.cpp`, `.json`) and automatically set the language state.

**Testing Strategy:**

- Ensure `ThemeManager` correctly applies the standard `ImGuiColorTextEdit` language definitions when invoked.
