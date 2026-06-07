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
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

# 3. Fetch ImGuiColorTextEdit
FetchContent_Declare(textedit GIT_REPOSITORY https://github.com/BalazsJako/ImGuiColorTextEdit.git GIT_TAG master)
FetchContent_GetProperties(textedit)
if(NOT textedit_POPULATED)
    FetchContent_Populate(textedit)
    # Patch TextEditor.cpp for ImGui 1.90+ compatibility
    file(READ "${textedit_SOURCE_DIR}/TextEditor.cpp" TEXT_EDITOR_CPP)
    string(REPLACE "ImGui::PushItemFlag(ImGuiItemFlags_NoTabStop, false);" "/* patched no tab stop */" TEXT_EDITOR_CPP "${TEXT_EDITOR_CPP}")
    string(REPLACE "ImGui::PopItemFlag();" "/* patched pop item flag */" TEXT_EDITOR_CPP "${TEXT_EDITOR_CPP}")
    file(WRITE "${textedit_SOURCE_DIR}/TextEditor.cpp" "${TEXT_EDITOR_CPP}")
endif()

# 4. Fetch Portable File Dialogs
FetchContent_Declare(pfd GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs.git GIT_TAG main)
FetchContent_GetProperties(pfd)
if(NOT pfd_POPULATED)
    FetchContent_Populate(pfd)
endif()

# 5. Fetch stb (image loader)
FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG master)
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
file(GLOB APP_SOURCES "src/*.cpp")
add_executable(FastNotepad WIN32
    ${APP_SOURCES}
    ${textedit_SOURCE_DIR}/TextEditor.cpp
)

# Make sure our app can find the TextEditor.h header file
target_include_directories(FastNotepad PRIVATE 
    src
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
