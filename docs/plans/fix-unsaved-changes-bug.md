# Plan: Fix Unsaved Changes Bug When Closing Tabs and App

## Bug Analysis

The user reported: "when i try to close the app by pressing on the X, the unsaved changes warning popup doesn't popup".

Upon investigating `src/core/NotepadApp.cpp`, I found **two critical bugs** related to closing tabs and the app with unsaved changes.

### Bug 1: The OS Window Close Event Bypasses the Unsaved Changes Check

The main loop condition is:

```cpp
while (!m_windowCtx.ShouldClose() || m_state.triggerExit)
```

If the user clicks the "X" button on the OS window, `m_windowCtx.ShouldClose()` becomes `true` during `m_windowCtx.PollEvents()`. On the next iteration of the `while` loop, `!m_windowCtx.ShouldClose()` evaluates to `false`. Since `m_state.triggerExit` is also `false`, the loop terminates **immediately**, completely skipping the inner block designed to intercept the close event:

```cpp
    if (m_windowCtx.ShouldClose()) {
      m_windowCtx.SetShouldClose(false);
      m_state.triggerExit = true;
    }
```

This causes the app to instantly close, deleting any unsaved changes without showing the warning modal.

### Bug 2: Closing a Background Dirty Tab Saves the Wrong Tab

When closing a specific tab via its "X" button, the code checks if the tab is dirty. If it is, it sets `pendingAction = CloseTab` and shows the "Unsaved Changes" modal. However, **it does not force the clicked tab to become the active tab**.
Because of this:

1. The modal pops up, but any action ("Save" or "Don't Save") operates on `GetActiveTab()`, which is still the previously active tab, not the background tab you intended to close!
2. If you click "Don't Save", the `isDirty` flag of your **active tab** is cleared, and the background tab is closed (losing its changes).
3. If you click "Save", it attempts to save the active tab, and then closes the background tab (losing its changes).

## Proposed Fix

We will fix both issues in `src/core/NotepadApp.cpp`:

1. **Fix OS Close Event:** Introduce a `bool running = true;` variable for the `while` loop. This ensures the loop always enters and correctly intercepts the `m_windowCtx.ShouldClose()` flag, triggering the exit flow. When it is safe to exit (`!hasDirty`), we set `running = false;` to gracefully break the loop.
2. **Fix Background Tab Close:** Add `m_state.forceSelectTab = i;` when a dirty tab is closed via its "X" button, ensuring it becomes the active tab before the user interacts with the save prompt.

### Task List

- [ ] Modify `src/core/NotepadApp.cpp` `Run()` to use `bool running = true;` for the main loop.
- [ ] Modify the `!hasDirty` condition in the `triggerExit` block to set `running = false;`.
- [ ] Add `m_state.forceSelectTab = i;` at line 243 for the tab close button logic.
- [ ] Compile the project (`cmake --build .`).
- [ ] Run the tests (`./scripts/run-tests.sh`) to ensure no regressions.

## Unit Testing (Sketch)

Although this is a UI-coupled logic inside the `NotepadApp::Run()` and `NotepadApp::RenderEditor()` loops which do not have isolated tests, the Catch2 testing sketch for state management would conceptually look like this:

```cpp
TEST_CASE("App Close intercepts OS close events correctly", "[NotepadApp]") {
    // Given an AppState with dirty tabs
    // When ShouldClose is true
    // Then the app must set triggerExit = true and not terminate the loop
}
```

Since the `NotepadApp` class handles this inline within ImGui calls and GLFW window events, we will rely on manual verification and the existing unit test suite to ensure no regressions occur.
