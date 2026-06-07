# Fast Notepad

A compact, cross-platform text editor built with C++ and Dear ImGui. It focuses on quick startup, a minimal UI, and practical editing tools.

## Getting started

Unix:

```bash
mkdir build
cd build
cmake ..
make
./FastNotepad
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


## Features

- Minimal top menu bar with File, Edit, and View menus
- New, Open, Save, Save As, and Exit actions
- Undo/Redo and clipboard actions (Cut/Copy/Paste)
- Markdown syntax highlighting toggle (regex-based)
- Light/Dark theme toggle
- Zoom in/out/reset for editor font size
- Native file dialogs via portable-file-dialogs
- Window title updates to show the active file

## Keyboard shortcuts

- Ctrl+O: Open
- Ctrl+S: Save
- Ctrl+Shift+S: Save As
- Ctrl+Z / Ctrl+Y: Undo / Redo
- Ctrl+X / Ctrl+C / Ctrl+V: Cut / Copy / Paste
- Ctrl+F: Find
- Ctrl+Mouse Wheel: Zoom
- Ctrl++ / Ctrl+- / Ctrl+0: Zoom In / Zoom Out / Reset Zoom

## Technology stack

- C++
- CMake
- GLFW
- OpenGL 3.3 Core
- Dear ImGui
- ImGuiColorTextEdit (TextEditor)
- portable-file-dialogs
- stb_image (window icon on Linux)
