cmake_minimum_required(VERSION 3.14)
project(FastNotepad CXX)

# Force C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automatically download and build GLFW (Lightweight windowing)
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
# Disable GLFW extras to keep compilation fast
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# Find system OpenGL
find_package(OpenGL REQUIRED)

# Define our executable
add_executable(FastNotepad src/main.cpp)

# Link the libraries to our app
target_link_libraries(FastNotepad PRIVATE glfw ${OPENGL_LIBRARIES})