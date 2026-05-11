cmake_minimum_required(VERSION 3.14)
project(CompactEditor CXX)

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

# 4. Create ImGui Static Library
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

# 5. Define our executable (Now including TextEditor.cpp)
add_executable(CompactEditor 
    src/main.cpp 
    ${textedit_SOURCE_DIR}/TextEditor.cpp
)

# Make sure our app can find the TextEditor.h header file
target_include_directories(CompactEditor PRIVATE ${textedit_SOURCE_DIR} ${imgui_SOURCE_DIR})
target_link_libraries(CompactEditor PRIVATE glfw ImGuiLib ${OPENGL_LIBRARIES})