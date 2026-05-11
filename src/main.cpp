#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "TextEditor.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "portable-file-dialogs.h"

std::string currentFilePath = "";

void LoadFile(const std::string& filepath, TextEditor& editor) {
    std::ifstream file(filepath);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        editor.SetText(buffer.str());
        currentFilePath = filepath;
    }
}

void SaveFile(const std::string& filepath, TextEditor& editor) {
    std::ofstream file(filepath);
    if (file.is_open()) {
        file << editor.GetText();
        currentFilePath = filepath;
    }
}

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
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    TextEditor editor;
    editor.SetPalette(TextEditor::GetDarkPalette());
    editor.SetText("Start typing or open a file...");

    // State flags for file operations
    bool triggerOpen = false;
    bool triggerSaveAs = false;
    bool triggerSave = false;

    std::string lastFilePath = "UNINITIALIZED";  // Force an update on the first frame

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // --- ONLY UPDATE TITLE IF THE FILE CHANGED ---
        if (currentFilePath != lastFilePath) {
            std::string title = currentFilePath.empty() ? "Compact Editor - Untitled" : "Compact Editor - " + currentFilePath;
            glfwSetWindowTitle(window, title.c_str());
            lastFilePath = currentFilePath;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false)) triggerOpen = true;
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
            if (io.KeyShift) {
                triggerSaveAs = true;
            } else {
                triggerSave = true;
            }
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    editor.SetText("");
                    currentFilePath = "";
                }

                // Set flags instead of blocking the UI thread immediately
                if (ImGui::MenuItem("Open", "Ctrl+O")) {
                    triggerOpen = true;
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    triggerSave = true;
                }
                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {
                    triggerSaveAs = true;
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z", nullptr, editor.CanUndo())) {
                    editor.Undo();
                }
                if (ImGui::MenuItem("Redo", "Ctrl+Y", nullptr, editor.CanRedo())) {
                    editor.Redo();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "Ctrl+X", nullptr, editor.HasSelection())) {
                    editor.Cut();
                }
                if (ImGui::MenuItem("Copy", "Ctrl+C", nullptr, editor.HasSelection())) {
                    editor.Copy();
                }
                if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                    editor.Paste();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        // ADDED: ImGuiWindowFlags_NoBringToFrontOnFocus (Fixes the flickering conflict)
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("MainWorkspace", nullptr, window_flags);
        editor.Render("TextEditor");
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Render();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        // --- HANDLE FILE DIALOGS AFTER RENDERING ---
        // This ensures the UI doesn't freeze or glitch while the OS window is open
        if (triggerOpen) {
            auto f = pfd::open_file("Choose text file", ".", {"Text Files", "*.txt *.md *.cpp *.h", "All Files", "*"});
            if (!f.result().empty()) {
                LoadFile(f.result()[0], editor);
            }
            triggerOpen = false;
        }
        if (triggerSave) {
            if (currentFilePath.empty()) {
                triggerSaveAs = true;
            } else {
                SaveFile(currentFilePath, editor);
            }
            triggerSave = false;
        }
        if (triggerSaveAs) {
            auto f = pfd::save_file("Save file", ".", {"Text Files", "*.txt *.md"});
            if (!f.result().empty()) {
                SaveFile(f.result(), editor);
            }
            triggerSaveAs = false;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}