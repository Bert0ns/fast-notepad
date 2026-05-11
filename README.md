# Ultra-Compact C++ Text Editor

## Getting started

Unix:

```bash
mkdir build
cd build
cmake ..
make
./FastNotepad
```

Windows:

```dos
mkdir build
cd build
cmake ..
cmake --build .
Debug\FastNotepad.exe
```

## Project Specifications 

### 1. Overview

A blazing-fast, ultra-lightweight text editor designed to replicate the simplicity of Windows 10 Notepad but with modern elegance, cross-platform compatibility, and Markdown support.

### 2. Core Requirements

* **Performance:** Instant startup, minimal memory footprint.
* **File Support:** Opens all types of *text* files. Normalizes line endings (Windows `\r\n` vs Linux `\n`). Defaults to UTF-8.
* **Cross-Platform:** Native execution on Windows and Linux (Ubuntu).
* **UI:** Elegant, simple, distraction-free.

### 3. Technology Stack

* **Language:** C++
* **Build System:** CMake (for generating Windows/Linux builds from a single source).
* **Windowing & Input:** GLFW (lightweight, cross-platform).
* **Graphics API:** OpenGL (Core Profile 3+). Avoids Vulkan to eliminate boilerplate and maintain instant startup.
* **UI Framework:** Dear ImGui (immediate mode, extremely fast, highly customizable).
* **Text Editor Component:** ImGuiColorTextEdit (lightweight, supports syntax highlighting, built-in undo/redo).
* **File Dialogs:** `portable-file-dialogs` (single-header, triggers native OS dialogs without heavy dependencies).

### 4. UI Architecture

* **Layout:** Two main zones: a classic Top Menu Bar and a full-screen Main Workspace for text. No sidebar or file explorer.
* **Top Menu Bar:**
  * **File:** New, Open, Save, Save As, Exit.
  * **Edit:** Undo, Redo, Cut, Copy, Paste.
  * **View:** Markdown Highlighting (Toggle), Word Wrap (Toggle), Theme (Dark/Light).
* **Main Workspace:** Borderless ImGui window dynamically resizing to fill the OS window.

### 5. Markdown Implementation

* **Strategy:** "Option A" - Syntax Highlighting only.
* **How it works:** The underlying text remains raw text. A custom `LanguageDefinition` in `ImGuiColorTextEdit` parses the screen text in real-time during the render loop (e.g., turning `# Headers` a specific color). Zero impact on the saved file and near-zero memory footprint. Leaves the door open for full rendering in the future.

### 6. Next Steps / Action Items (Implementation Roadmap)

* [ ] Set up CMake project structure for cross-platform builds.
* [ ] Initialize GLFW window and OpenGL context.
* [ ] Integrate Dear ImGui and set up the basic render loop.
* [ ] Implement the Top Menu Bar UI layout.
* [ ] Integrate `ImGuiColorTextEdit` into the main workspace.
* [ ] Add file I/O operations (Open, Save) with `portable-file-dialogs`.
* [ ] Create the custom Markdown Language Definition for syntax highlighting.