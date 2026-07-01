# Implementation Plan: Phase 4 Bug Fixes and Code Quality Improvements

## Overview

This plan addresses the critical correctness bugs, performance bottlenecks, readability issues, and dead code identified during the full codebase review following Phase 4. Our goal is to stabilize the multi-tab and syntax highlighting features before proceeding to Phase 5.

## Architecture Decisions

- **App Exit Flow:** Instead of immediately closing the window context upon `triggerExit`, the app will iterate over all tabs and prompt the user to save any dirty tabs individually by leveraging the existing `showUnsavedChangesModal`.
- **ReplaceAll Optimization:** We will extract the full text from the editor once, perform string replacement in standard C++ to prevent $O(N^2)$ reallocation of lines, and then overwrite the buffer. While this clears the undo history for the "Replace All" action, it prevents memory spikes and thousands of polluted undo states.
- **Dead Code Cleanup:** Any remaining synchronous file handling mechanisms from Phase 1 will be removed to ensure all file I/O routes through the async futures pattern introduced in Phase 2.

## Task List

### Phase 1: Critical Correctness (Data Loss Prevention)

- [ ] **Task 1: Fix `currentFilePath` clearing on file load**
  - **Description:** When an async file load completes successfully, `tab->currentFilePath` is incorrectly assigned the value of `tab->pendingFilePath`, which is empty during a file open operation. This strips the tab of its file association.
  - **Acceptance criteria:**
    - Opening a file retains the correct file path (tab title shows the file name, not "Untitled").
    - Saving a newly opened file does not trigger "Save As".
  - **Verification:**
    - Manual check: Open an existing file, verify the tab title, modify it, and hit Ctrl+S. It should save without a prompt.
  - **Dependencies:** None
  - **Files likely touched:** `src/core/NotepadApp.cpp`
  - **Estimated scope:** XS (1 file)

- [ ] **Task 2: Intercept App Exit for Dirty Tabs**
  - **Description:** Closing the app window currently bypasses the dirty-state check for all tabs, leading to data loss if multiple tabs have unsaved changes.
  - **Acceptance criteria:**
    - Clicking exit (or closing the window) iterates through tabs and triggers the unsaved changes modal for the first dirty tab found.
    - The app only exits when all tabs are clean or the user explicitly chooses "Don't Save".
  - **Verification:**
    - Manual check: Open two tabs, make both dirty, click exit. Ensure the app prompts for the first tab, and then the second tab, before actually exiting.
  - **Dependencies:** None
  - **Files likely touched:** `src/core/NotepadApp.cpp`
  - **Estimated scope:** S (1 file)

### Checkpoint: Correctness

- [ ] Tests pass, builds clean
- [ ] No data loss occurs during file loading or app exit.

### Phase 2: Performance and UX

- [ ] **Task 3: Optimize `FindPanel::ReplaceAll`**
  - **Description:** The current `ReplaceAll` implementation calls `FindNextMatch` in a loop, which copies the entire document text buffer into a `std::vector<std::string>` each time. It also pushes an undo record for every single replacement. We will rewrite it to use `editor.GetText()`, perform a standard `std::string::replace` loop, and update the editor with `editor.SetText()`.
  - **Acceptance criteria:**
    - `ReplaceAll` performs instantly even on large files with thousands of matches.
    - Memory usage does not spike during `ReplaceAll`.
  - **Verification:**
    - Tests pass: `ctest -R FindPanel`
    - Manual check: Generate a large text file, replace a common character, verify it doesn't hang.
  - **Dependencies:** None
  - **Files likely touched:** `src/ui/FindPanel.cpp`
  - **Estimated scope:** S (1 file)

- [ ] **Task 4: Fix Light Theme Highlighter Contrast**
  - **Description:** The current selection highlighter color in light mode (`0x80ffb080`) blends too closely with regular text, making it illegible.
  - **Acceptance criteria:**
    - Selection color is changed to a distinct, highly visible shade (e.g., Sky Blue `0x80ebce87`).
  - **Verification:**
    - Manual check: Switch to light theme, select some text, verify readability.
  - **Dependencies:** None
  - **Files likely touched:** `src/ui/ThemeManager.cpp`
  - **Estimated scope:** XS (1 file)

### Phase 3: Cleanup

- [ ] **Task 5: Remove synchronous FileHandler dead code**
  - **Description:** `FileHandler::HandleDialogs`, `FileHandler::LoadFile`, and `FileHandler::SaveFile` are unused remnants from Phase 1.
  - **Acceptance criteria:**
    - The dead code is removed from the header and implementation files.
  - **Verification:**
    - Tests pass: `ctest`
    - Build succeeds: `./wsl-build.sh`
  - **Dependencies:** None
  - **Files likely touched:** `src/io/FileHandler.h`, `src/io/FileHandler.cpp`
  - **Estimated scope:** S (2 files)

### Checkpoint: Complete

- [ ] All acceptance criteria met
- [ ] Ready for human review

## Risks and Mitigations

| Risk                                      | Impact | Mitigation                                                                                                                                                       |
| ----------------------------------------- | ------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Modifying Exit Flow introduces edge cases | High   | Keep the logic simple: loop through tabs, break on the first dirty one, set `pendingAction = Exit` and trigger the modal. Rely on existing pending action logic. |
| `SetText` clears undo history             | Low    | Acknowledge this UX tradeoff for `ReplaceAll` performance. Most text editors clear or bulk-wrap history for heavy programmatic replacements anyway.              |
