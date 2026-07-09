## Goal Description

The goal is to improve the build time of the Fast Notepad project. Currently, the project takes longer than necessary to configure and build due to full git clones of dependencies, missing compiler caching, lack of parallelization in test scripts, and unconditional building of testing dependencies.

## Proposed Changes

### CMakeLists.txt

- Add `GIT_SHALLOW ON` to all `FetchContent_Declare` blocks (`glfw`, `imgui`, `textedit`, `pfd`, `stb`, `Catch2`). This ensures only the latest commit is downloaded instead of the entire git history, drastically reducing dependency fetch time.
- Add support for `ccache` (if installed on the user's system) to cache compiled object files, speeding up subsequent builds.
- Wrap Catch2 fetch and testing targets inside an `if(BUILD_TESTING)` block. Testing targets will no longer be fetched or built unconditionally when tests are not explicitly requested.
- Define `BUILD_TESTING` as an option defaulting to `OFF`.

#### [MODIFY] CMakeLists.txt

```cmake
# ...
option(BUILD_TESTING "Build the testing tree" OFF)

# Add ccache support
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# 1. Fetch GLFW
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.3.8 GIT_SHALLOW ON)
# ...
# 2. Fetch Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.4
    GIT_SHALLOW    ON
)
# ...
# 3. Fetch ImGuiColorTextEdit
FetchContent_Declare(
    textedit
    GIT_REPOSITORY https://github.com/Bert0ns/ImGuiColorTextEdit.git
    GIT_TAG        bert0ns/html-support
    GIT_SHALLOW    ON
)
# ...
# 4. Fetch Portable File Dialogs
FetchContent_Declare(pfd GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs.git GIT_TAG c12ea8c9a727f5320a2b4570aee863bbede2a204 GIT_SHALLOW ON)
# ...
# 5. Fetch stb (image loader)
FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG 31c1ad37456438565541f4919958214b6e762fb4 GIT_SHALLOW ON)
# ...

# 8. Testing setup
if(BUILD_TESTING)
    enable_testing()
    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG        v3.5.2
      GIT_SHALLOW    ON
    )
    FetchContent_MakeAvailable(Catch2)
    # ... tests definition ...
    catch_discover_tests(fast_notepad_tests)
endif()
```

### scripts/run-tests.sh

- Modify the build command to use `--parallel` to leverage multiple CPU cores during the build step.

#### [MODIFY] scripts/run-tests.sh

```diff
-cmake --build . --target fast_notepad_tests
+cmake --build . --parallel --target fast_notepad_tests
```

## Verification Plan

### Automated Tests

Run the test script to verify parallel building and that the tests still pass:

```bash
./scripts/run-tests.sh
```

### Manual Verification

1. Clean the build directory (`rm -rf build` and `rm -rf build-tests`).
2. Run `cmake -B build -S .`. Observe that the configuration step is much faster due to shallow cloning and Catch2 is skipped.
3. Verify that `cmake --build build` works for the main executable without downloading Catch2.
4. Verify that subsequent builds are near-instant or much faster due to `ccache` (if available on the system).
