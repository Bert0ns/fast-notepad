cmake_minimum_required(VERSION 3.14)
project(FastNotepad CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Fetch GLFW
include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.3.8)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

# 2. Fetch Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.4
)
FetchContent_MakeAvailable(imgui)

# 3. Fetch ImGuiColorTextEdit
FetchContent_Declare(textedit GIT_REPOSITORY https://github.com/BalazsJako/ImGuiColorTextEdit.git GIT_TAG ca2f9f1462e3b60e56351bc466acda448c5ea50d)
FetchContent_MakeAvailable(textedit)
# Patch TextEditor.cpp for ImGui 1.90+ compatibility
file(READ "${textedit_SOURCE_DIR}/TextEditor.cpp" TEXT_EDITOR_CPP)
string(REPLACE "ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, false);" "/* patched no tab stop */" TEXT_EDITOR_CPP "${TEXT_EDITOR_CPP}")
string(REPLACE "ImGui::PopItemFlag();" "/* patched pop item flag */" TEXT_EDITOR_CPP "${TEXT_EDITOR_CPP}")

set(SCROLL_ORIG "	if (pos.mLine < top)
		ImGui::SetScrollY(std::max(0.0f, (pos.mLine - 1) * mCharAdvance.y));
	if (pos.mLine > bottom - 4)
		ImGui::SetScrollY(std::max(0.0f, (pos.mLine + 4) * mCharAdvance.y - height));
	if (len + mTextStart < left + 4)
		ImGui::SetScrollX(std::max(0.0f, len + mTextStart - 4));
	if (len + mTextStart > right - 4)
		ImGui::SetScrollX(std::max(0.0f, len + mTextStart + 4 - width));")

set(SCROLL_NEW "	float padY = std::min(2.0f * mCharAdvance.y, height * 0.25f);
	float padX = std::min(4.0f * mCharAdvance.x, width * 0.25f);
	
	float cursorPosY = pos.mLine * mCharAdvance.y;
	float cursorPosX = len + mTextStart;
	
	if (cursorPosY < scrollY + padY)
		ImGui::SetScrollY(std::max(0.0f, cursorPosY - padY));
	if (cursorPosY + mCharAdvance.y > scrollY + height - padY)
		ImGui::SetScrollY(std::max(0.0f, cursorPosY + mCharAdvance.y - height + padY));
		
	if (cursorPosX < scrollX + padX)
		ImGui::SetScrollX(std::max(0.0f, cursorPosX - padX));
	if (cursorPosX > scrollX + width - padX)
		ImGui::SetScrollX(std::max(0.0f, cursorPosX - width + padX));")

string(REPLACE "${SCROLL_ORIG}" "${SCROLL_NEW}" TEXT_EDITOR_CPP "${TEXT_EDITOR_CPP}")

file(WRITE "${textedit_SOURCE_DIR}/TextEditor.cpp" "${TEXT_EDITOR_CPP}")

# 4. Fetch Portable File Dialogs
FetchContent_Declare(pfd GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs.git GIT_TAG c12ea8c9a727f5320a2b4570aee863bbede2a204)
FetchContent_GetProperties(pfd)
if(NOT pfd_POPULATED)
    FetchContent_Populate(pfd)
endif()

# 5. Fetch stb (image loader)
FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG 31c1ad37456438565541f4919958214b6e762fb4)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()

# 6. Create ImGui Static Library
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(ImGuiLib STATIC ${IMGUI_SOURCES})
target_include_directories(ImGuiLib PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(ImGuiLib PRIVATE glfw ${OPENGL_LIBRARIES})

# 7. Define our executable (Now including TextEditor.cpp)
file(GLOB_RECURSE APP_SOURCES "src/*.cpp")
add_executable(FastNotepad WIN32
    ${APP_SOURCES}
    ${textedit_SOURCE_DIR}/TextEditor.cpp
)

# Make sure our app can find the TextEditor.h header file
target_include_directories(FastNotepad PRIVATE 
    src
    src/core
    src/ui
    src/io
    src/utils
    ${textedit_SOURCE_DIR} 
    ${imgui_SOURCE_DIR} 
    ${pfd_SOURCE_DIR}
    ${stb_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
)
target_link_libraries(FastNotepad PRIVATE glfw ImGuiLib ${OPENGL_LIBRARIES})

# Embed the application icon on Windows.
if(WIN32)
    set(APP_ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/assets/app.rc")
    target_sources(FastNotepad PRIVATE ${APP_ICON_RESOURCE})
endif()

if(MSVC)
    # Build as a GUI app without requiring a WinMain entry point.
    target_link_options(FastNotepad PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
endif()

# 8. Testing setup
enable_testing()
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(fast_notepad_tests 
    tests/test_session_manager.cpp 
    tests/test_file_handler.cpp
    tests/test_theme_manager.cpp
    tests/test_platform.cpp
    tests/test_find_panel.cpp
    tests/test_shortcut_manager.cpp
    tests/test_menu_bar.cpp
    src/core/SessionManager.cpp
    src/io/FileHandler.cpp
    src/ui/ThemeManager.cpp
    src/utils/Platform.cpp
    src/ui/FindPanel.cpp
    src/utils/Utf8Utils.cpp
    src/io/NativeFileDialog.cpp
    src/core/WindowContext.cpp
    src/core/ShortcutManager.cpp
    src/ui/MenuBar.cpp
    ${textedit_SOURCE_DIR}/TextEditor.cpp
)
target_include_directories(fast_notepad_tests PRIVATE 
    src
    src/core
    src/ui
    src/io
    src/utils
    ${textedit_SOURCE_DIR} 
    ${imgui_SOURCE_DIR} 
    ${pfd_SOURCE_DIR}
    ${stb_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
)
target_link_libraries(fast_notepad_tests PRIVATE 
    Catch2::Catch2WithMain
    ImGuiLib
    glfw
    ${OPENGL_LIBRARIES}
)

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(fast_notepad_tests PRIVATE --coverage -O0 -g)
    target_link_options(fast_notepad_tests PRIVATE --coverage)
endif()

include(Catch)
catch_discover_tests(fast_notepad_tests)

