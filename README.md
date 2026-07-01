# Fast Notepad

A compact, cross-platform text editor built with C++ and Dear ImGui. It focuses on quick startup, a minimal UI, and practical editing tools.

## Getting started

Unix:

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
echo "run the app with ./build/FastNotepad"
```

If an error shows up talnking about X11, you are missing the X11 + OpenGL dev packages, install them:

```bash
sudo apt update
sudo apt install -y \
  xorg-dev \
  libx11-dev \
  libxrandr-dev \
  libxinerama-dev \
  libxcursor-dev \
  libxi-dev \
  libgl1-mesa-dev
```

Windows:

```dos
mkdir build
cd build
cmake ..
cmake --build .
Debug\FastNotepad.exe
```

Other useful command to build the release version: `CMake: Select Variant`

You can also run `./scripts/build-release.sh`

## Testing

To run the automated tests locally using Catch2, you need to configure CMake with testing enabled and then use `ctest`:

```bash
mkdir build-tests
cd build-tests
cmake .. -DBUILD_TESTING=ON
cmake --build . --target fast_notepad_tests
ctest --output-on-failure
```

You can also run `./scripts/run-tests.sh`

## Releases

Releases are completely automated via GitHub Actions. To trigger a new release:

1. Create and push a new Git tag starting with `v` (e.g., `v1.0.0`):
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```
2. The GitHub Action will automatically build binaries for Windows, macOS, and Linux, and create a GitHub Release with the attached files.
3. You can also manually trigger a build from the **Actions** tab on GitHub by selecting the **Release** workflow and clicking **Run workflow**.

## Features

- **Multi-Tab Support:** Open, edit, and manage multiple files concurrently.
- **Syntax Highlighting:** Automatically detects and highlights C++, JSON, GLSL, Lua, and Markdown files.
- **Find & Replace:** Search for text and replace it iteratively or all at once.
- **Unsaved Changes Protection:** Prevents accidental data loss by prompting you before closing dirty tabs or exiting.
- **Session Persistence:** Remembers your open tabs and settings (theme, window size) across restarts.
- **Light/Dark Themes:** Beautiful, high-contrast themes optimized for readability.
- **Quick Zooming:** Scale editor font size easily using mouse wheel or keyboard shortcuts.
- **Native File Dialogs:** Seamless OS integration using portable-file-dialogs.

## Keyboard shortcuts

- **Ctrl+N:** New Tab
- **Ctrl+O:** Open File
- **Ctrl+S:** Save
- **Ctrl+Shift+S:** Save As
- **Ctrl+Z / Ctrl+Y:** Undo / Redo
- **Ctrl+X / Ctrl+C / Ctrl+V:** Cut / Copy / Paste
- **Ctrl+F:** Find
- **Ctrl+H:** Find & Replace
- **Esc:** Close Find & Replace panel
- **Ctrl+Mouse Wheel:** Zoom In / Out
- **Ctrl++ / Ctrl+- / Ctrl+0:** Zoom In / Zoom Out / Reset Zoom

## Technology stack

- C++
- CMake
- GLFW
- OpenGL 3.3 Core
- Dear ImGui
- ImGuiColorTextEdit (TextEditor)
- portable-file-dialogs
- stb_image (window icon on Linux)
