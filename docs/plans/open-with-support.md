# Plan: Support "Open with" (Command Line Arguments)

## Objective

Enable FastNotepad to accept file paths via command line arguments so that it seamlessly integrates with the Windows "Open with" context menu and terminal usage.

## Analysis

When you use "Open with" on Windows (or pass arguments via terminal on Linux/macOS), the OS executes the application with the file paths as command-line arguments (e.g., `FastNotepad.exe "C:\path\to\file.txt"`).
Currently, `main.cpp` does not capture `argc` and `argv`, and `NotepadApp::Init()` doesn't process them.

## Implementation Steps

- [x] **1. Modify `main.cpp`**
  - Change the signature to `int main(int argc, char** argv)`.
  - Pass these arguments into the `NotepadApp` class.

- [x] **2. Update `NotepadApp::Init(int argc, char** argv)`\*\*
  - Parse the command-line arguments (skipping `argv[0]` which is the executable path).
  - Modify the startup logic:
    - The app will still attempt to load the previous session (the restored tabs).
    - For every file path passed in the arguments, we will check if it's already open in the session.
    - If it's already open, we focus on that tab.
    - If it's not open, we create a new tab for it and load the file asynchronously.
    - We focus the first file that was passed via the command line.

- [x] **3. Handle Edge Cases**
  - If the application starts with CLI arguments but _no_ session exists, it shouldn't show the default "Start typing..." tab. It should only show the files passed via the command line.

- [x] **4. Create Windows Registry Guide / Script**
  - Create a `scripts/windows-context-menu.reg` template file or a README section. Windows requires registry keys under `HKEY_CLASSES_ROOT\*\shell` to show "Open with FastNotepad" in the right-click menu. This script will allow you to quickly register the app.

- [x] **5. Add Unit Tests**
  - We can mock `argc` and `argv` and pass them to a test wrapper or test the argument parsing logic to ensure coverage remains high.

## Questions for you

1. Do you agree with the behavior that opening a file via "Open with" will **also** restore your previous session tabs alongside the new file? (This is how VS Code handles it). Or should it _only_ open the requested file?
