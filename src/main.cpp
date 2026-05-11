#include <GLFW/glfw3.h>

#include <iostream>

#include "TextEditor.h"  // Our new text editor component!
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(900, 600, "Compact Editor", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // --- INITIALIZE THE TEXT EDITOR ---
    TextEditor editor;
    editor.SetPalette(TextEditor::GetDarkPalette());
    // For now, we will just use a blank language definition.
    // We will build the custom Markdown one later.
    editor.SetText("Welcome to your ultra-compact text editor.\nStart typing here...");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. --- DRAW THE TOP MENU BAR ---
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    editor.SetText("");
                }
                if (ImGui::MenuItem("Open", "Ctrl+O")) { /* TODO */
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { /* TODO */
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                    editor.Undo();
                }
                if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                    editor.Redo();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "Ctrl+X")) {
                    editor.Cut();
                }
                if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                    editor.Copy();
                }
                if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                    editor.Paste();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                // We will implement these toggles in the next steps
                ImGui::MenuItem("Markdown Highlighting");
                ImGui::MenuItem("Word Wrap");
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // 2. --- DRAW THE MAIN WORKSPACE ---
        // Get the area of the screen *below* the menu bar
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        // Force the window to be borderless, static, and fill the screen
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        // Create the window and render the editor inside it
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));  // Remove padding around editor
        ImGui::Begin("MainWorkspace", nullptr, window_flags);
        editor.Render("TextEditor");
        ImGui::End();
        ImGui::PopStyleVar();

        // --- RENDER EVERYTHING TO THE SCREEN ---
        ImGui::Render();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}