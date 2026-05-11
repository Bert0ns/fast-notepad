#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "TextEditor.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "portable-file-dialogs.h"

#define DEFAULT_FONT_INDEX (3)

std::string currentFilePath = "";

TextEditor::LanguageDefinition GetMarkdownDefinition() {
    TextEditor::LanguageDefinition lang;
    lang.mName = "Markdown";

    lang.mTokenRegexStrings.push_back(std::make_pair("`[^`]+`", TextEditor::PaletteIndex::String));

    lang.mTokenRegexStrings.push_back(std::make_pair("\\*\\*[^*]+\\*\\*", TextEditor::PaletteIndex::Keyword));

    lang.mTokenRegexStrings.push_back(std::make_pair("\\*[^*]+\\*", TextEditor::PaletteIndex::KnownIdentifier));

    lang.mTokenRegexStrings.push_back(std::make_pair("^[ \\t]*#+", TextEditor::PaletteIndex::Preprocessor));

    lang.mTokenRegexStrings.push_back(std::make_pair("^[ \\t]*>", TextEditor::PaletteIndex::Comment));

    // Use a sentinel to avoid empty comment matches in the parser.
    lang.mCommentStart = "\x01";
    lang.mCommentEnd = "\x01";
    lang.mSingleLineComment = "\x01";
    lang.mPreprocChar = 0;
    lang.mAutoIndentation = false;
    lang.mCaseSensitive = true;

    return lang;
}

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

    GLFWwindow* window = glfwCreateWindow(900, 600, "Fast Notepad", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    std::string currentFilePath = "";
    std::vector<ImFont*> editorFonts;
    int currentFontIndex = DEFAULT_FONT_INDEX;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();

    const char* fontPath = nullptr;
    std::ifstream f_win("C:\\Windows\\Fonts\\consola.ttf");
    std::ifstream f_lin("/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf");
    if (f_win.good())
        fontPath = "C:\\Windows\\Fonts\\consola.ttf";
    else if (f_lin.good())
        fontPath = "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";

    // Load monospace font sizes for zooming.
    if (fontPath) {
        for (int size = 14; size <= 32; size += 2) {
            editorFonts.push_back(io.Fonts->AddFontFromFileTTF(fontPath, size));
        }
    } else {
        for (int i = 0; i < 10; ++i) editorFonts.push_back(nullptr);
    }

    TextEditor::LanguageDefinition markdownDef = GetMarkdownDefinition();

    TextEditor editor;
    editor.SetPalette(TextEditor::GetDarkPalette());
    editor.SetText("_Start *typing* or open a file..._");

    TextEditor::LanguageDefinition plainTextDef;
    plainTextDef.mName = "Plain Text";
    plainTextDef.mCommentStart = "\x01";
    plainTextDef.mCommentEnd = "\x01";
    plainTextDef.mSingleLineComment = "\x01";
    plainTextDef.mPreprocChar = 0;
    plainTextDef.mAutoIndentation = false;
    editor.SetLanguageDefinition(plainTextDef);

    bool triggerOpen = false;
    bool triggerSaveAs = false;
    bool triggerSave = false;

    bool enableMarkdown = true;

    if (enableMarkdown) {
        editor.SetLanguageDefinition(markdownDef);
    }

    std::string lastFilePath = "UNINITIALIZED";

    bool isDarkTheme = true;
    ImVec4 clear_color = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (currentFilePath != lastFilePath) {
            std::string title = currentFilePath.empty() ? "Fast Notepad - Untitled" : "Fast Notepad - " + currentFilePath;
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
        if (io.KeyCtrl && io.MouseWheel != 0.0f) {
            if (io.MouseWheel > 0)
                currentFontIndex++;
            else
                currentFontIndex--;
        }
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Equal, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd, false) || ImGui::IsKeyPressed(ImGuiKey_RightBracket, false))) currentFontIndex++;
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Minus, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract, false))) currentFontIndex--;
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_0, false)) currentFontIndex = DEFAULT_FONT_INDEX;

        if (currentFontIndex < 0) currentFontIndex = 0;
        if (currentFontIndex >= (int)editorFonts.size()) currentFontIndex = (int)editorFonts.size() - 1;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    editor.SetText("");
                    currentFilePath = "";
                }

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
            if (ImGui::BeginMenu("View")) {
                bool prevMarkdownState = enableMarkdown;
                ImGui::MenuItem("Markdown Highlighting", nullptr, &enableMarkdown);

                if (enableMarkdown != prevMarkdownState) {
                    if (enableMarkdown) {
                        editor.SetLanguageDefinition(markdownDef);
                        printf("Markdown highlighting enabled.\n");
                    } else {
                        editor.SetLanguageDefinition(plainTextDef);
                        printf("Markdown highlighting disabled.\n");
                    }
                }

                ImGui::Separator();

                bool prevThemeState = isDarkTheme;
                ImGui::MenuItem("Dark Theme", nullptr, &isDarkTheme);
                if (isDarkTheme != prevThemeState) {
                    if (isDarkTheme) {
                        ImGui::StyleColorsDark();
                        editor.SetPalette(TextEditor::GetDarkPalette());
                        clear_color = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
                    } else {
                        ImGui::StyleColorsLight();
                        editor.SetPalette(TextEditor::GetLightPalette());
                        clear_color = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
                    }
                }

                if (ImGui::MenuItem("Zoom In", "Ctrl++")) {
                    currentFontIndex++;
                }
                if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
                    currentFontIndex--;
                }
                if (ImGui::MenuItem("Reset Zoom", "Ctrl+0")) {
                    currentFontIndex = DEFAULT_FONT_INDEX;
                }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("MainWorkspace", nullptr, window_flags);
        if (editorFonts[currentFontIndex] != nullptr) {
            ImGui::PushFont(editorFonts[currentFontIndex]);
        }

        editor.Render("TextEditor");

        if (editorFonts[currentFontIndex] != nullptr) {
            ImGui::PopFont();
        }
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Render();
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        // Run file dialogs after rendering to avoid UI blocking artifacts.
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