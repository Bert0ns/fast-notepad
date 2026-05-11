#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    // 1. Initialize the GLFW library
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // 2. Request an OpenGL 3.3 Core Profile context (Fast, modern, lightweight)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 3. Create the OS window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Compact Editor", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // 4. Make the window the current drawing context
    glfwMakeContextCurrent(window);

    // Enable vsync (Syncs drawing to your monitor's refresh rate to prevent screen tearing)
    glfwSwapInterval(1);

    // 5. The Main Application Loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen to a sleek dark grey background
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap the front and back buffers to display the new frame
        glfwSwapBuffers(window);

        // Listen for OS events (mouse clicks, keyboard typing, window resizing)
        glfwPollEvents();
    }

    // 6. Clean up and exit gracefully
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}