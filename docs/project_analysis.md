# Fast Notepad - Project Analysis & Improvement Report

After exploring the Fast Notepad codebase, several key areas have been identified where the application can be improved in terms of both **Code Quality** and **New Features**. The project has a solid, well-organized foundation using modern C++17, CMake (FetchContent), and Dear ImGui. Below are detailed observations and recommendations.

## 1. Code Quality & Architectural Improvements

### A. Dependency Management

- **Issue**: Several dependencies in `CMakeLists.txt` (`stb`, `pfd`, `ImGuiColorTextEdit`) use floating tags like `GIT_TAG master` or `GIT_TAG main`.
- **Improvement**: Pin these dependencies to specific commits or release tags. Floating tags can cause the build to break unexpectedly if the upstream repository introduces a breaking change.

### B. Error Handling & User Feedback

- **Issue**: In `src/io/FileHandler.cpp`, methods like `LoadFile` and `SaveFile` fail silently if a file cannot be opened or written to.
- **Improvement**: These methods should return `bool` or an error message (`std::optional<std::string>`). `NotepadApp` should catch these errors and display an ImGui modal/popup to inform the user (e.g., "Permission Denied" or "File Not Found").

### C. Refactoring `NotepadApp`

- **Issue**: `NotepadApp.cpp` is a God Class doing a bit too much: it handles the main GLFW loop, shortcut polling, menu bar rendering, and editor initialization.
- **Improvement**:
  - Extract the Menu Bar into a dedicated `MenuBar` class.
  - Move shortcut handling to a `ShortcutManager` or `InputHandler`.
  - This separation of concerns will keep `NotepadApp.cpp` lean as the app scales.

### D. File I/O Blocking

- **Issue**: File operations are synchronous.
- **Improvement**: For very large files or slow network drives, loading and saving will freeze the UI. Moving these operations to a background thread (using `std::async` or a simple worker thread) will keep the application responsive.

---

## 2. Proposed New Features

### A. Unsaved Changes Protection (Critical)

- **Feature**: Currently, if the user modifies a document and clicks "New" or closes the app, changes are lost or silently overwritten by the session state.
- **Implementation**: Hook into `ImGuiColorTextEdit::IsTextChanged()`. When exiting or opening a new file, if the text has changed, prompt the user with a modal: _"You have unsaved changes. Would you like to save them?"_

### B. Multi-Tab Support

- **Feature**: Allow opening multiple files at once.
- **Implementation**: Utilize ImGui's native `ImGui::BeginTabBar` and `ImGui::BeginTabItem`. `NotepadApp` would manage a `std::vector<EditorInstance>` instead of a single text editor, drastically improving usability.

### C. Extended Syntax Highlighting

- **Feature**: The app currently supports basic text and a Markdown toggle.
- **Implementation**: `ImGuiColorTextEdit` natively supports languages like C++, HLSL, GLSL, Lua, C, etc. Easily add a "Language" dropdown in the "View" menu to switch the active `LanguageDefinition`.

### D. Find & Replace

- **Feature**: The app has a `FindPanel`, but lacks a "Replace" functionality.
- **Implementation**: Extend `FindPanel` to include a "Replace With" text input and "Replace" / "Replace All" buttons.

### E. Settings Modal

- **Feature**: Instead of toggling themes and fonts exclusively through the View menu and shortcuts, add a dedicated Settings window.
- **Implementation**: A modal that lets users configure font size, default font family, tab width, light/dark mode, and word wrap preferences.
