# Plan: Increase Test Coverage

## Objective

Increase the overall test coverage of the Fast Notepad project by writing automated Catch2 tests for `WindowContext.cpp`, which currently sits at just 10% coverage.

## Analysis

- **`WindowContext.cpp` (10% coverage):** The file handles GLFW initialization, window creation, OpenGL context setup, and ImGui initialization. Since our CI and local test environment runs with an X11 server (or similar display server) available, we can safely invoke these real Window API calls within Catch2 tests without them failing.
- **`MenuBar.cpp` (16% coverage):** While a test file (`test_menu_bar.cpp`) already exists, ImGui's immediate mode paradigm dictates that `ImGui::BeginMenu("File")` returns `false` unless a user is physically interacting with the UI. Thus, the inner menu item logic is skipped during automated headless test runs. Writing a robust test for this would require an interaction simulator (like `ImGuiTestEngine`), which we want to avoid for lightweight dependencies. We will accept the low coverage here.
- **`NativeFileDialog.cpp` (0% coverage):** Testing this invokes real OS dialogs that block the process. We will intentionally skip testing this wrapper.

## Implementation Steps

- [x] **1. Create `tests/test_window_context.cpp`**
  - Implement a `TEST_CASE` for `WindowContext`.
  - Validate that `WindowContext::Init(800, 600, "Test")` successfully creates a window and returns `true`.
  - Validate `SetWindowTitle("New Title")` executes without throwing errors.
  - Validate `SetShouldClose(true)` triggers `ShouldClose() == true`.
  - Invoke `PollEvents()` and `SwapBuffers()` momentarily to guarantee the pipeline paths execute correctly and contribute to coverage.

- [x] **2. Update `CMakeLists.txt`**
  - Make sure `test_window_context.cpp` is included in the `fast_notepad_tests` target sources (it likely is automatically picked up via a glob, but we will verify this).

- [x] **3. Run tests**
  - Execute `./scripts/run-tests.sh` to confirm that the tests pass and the coverage of `WindowContext.cpp` drastically increases, improving the overall project coverage.
