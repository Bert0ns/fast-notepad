#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "TextEditor.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "portable-file-dialogs.h"

namespace {
constexpr int kDefaultFontIndex = 3;
constexpr int kFontMinSize = 14;
constexpr int kFontMaxSize = 32;
constexpr int kFontStep = 2;
constexpr int kFontSlots = ((kFontMaxSize - kFontMinSize) / kFontStep) + 1;
constexpr const char* kNoCommentSentinel = "\x01";
constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 600;
constexpr const char* kWindowTitle = "Fast Notepad";

struct AppState {
    std::string currentFilePath;
    std::string lastFilePath = "UNINITIALIZED";
    bool triggerOpen = false;
    bool triggerSaveAs = false;
    bool triggerSave = false;
    bool enableMarkdown = true;
    bool isDarkTheme = true;
};

void ApplyNoCommentParsing(TextEditor::LanguageDefinition& lang) {
    lang.mCommentStart = kNoCommentSentinel;
    lang.mCommentEnd = kNoCommentSentinel;
    lang.mSingleLineComment = kNoCommentSentinel;
    lang.mPreprocChar = 0;
    lang.mAutoIndentation = false;
}

const char* FindMonospaceFontPath() {
    std::ifstream f_win("C:\\Windows\\Fonts\\consola.ttf");
    if (f_win.good()) {
        return "C:\\Windows\\Fonts\\consola.ttf";
    }

    std::ifstream f_lin("/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf");
    if (f_lin.good()) {
        return "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";
    }

    return nullptr;
}

void LoadEditorFonts(ImGuiIO& io, std::vector<ImFont*>& editorFonts) {
    io.Fonts->AddFontDefault();
    const char* fontPath = FindMonospaceFontPath();
    if (fontPath) {
        for (int size = kFontMinSize; size <= kFontMaxSize; size += kFontStep) {
            editorFonts.push_back(io.Fonts->AddFontFromFileTTF(fontPath, size));
        }
        return;
    }

    for (int i = 0; i < kFontSlots; ++i) {
        editorFonts.push_back(nullptr);
    }
}

void SetMarkdownMode(bool enableMarkdown, TextEditor& editor, const TextEditor::LanguageDefinition& markdownDef, const TextEditor::LanguageDefinition& plainTextDef) {
    editor.SetLanguageDefinition(enableMarkdown ? markdownDef : plainTextDef);
}

void ApplyTheme(bool isDarkTheme, TextEditor& editor, ImVec4& clearColor) {
    if (isDarkTheme) {
        ImGui::StyleColorsDark();
        editor.SetPalette(TextEditor::GetDarkPalette());
        clearColor = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    } else {
        ImGui::StyleColorsLight();
        editor.SetPalette(TextEditor::GetLightPalette());
        clearColor = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    }
}

void ClampFontIndex(int& index, int maxSize) {
    if (index < 0) index = 0;
    if (index >= maxSize) index = maxSize - 1;
}
}  // namespace

TextEditor::LanguageDefinition GetMarkdownDefinition() {
    TextEditor::LanguageDefinition lang;
    lang.mName = "Markdown";

    lang.mTokenRegexStrings.push_back(std::make_pair("`[^`]+`", TextEditor::PaletteIndex::String));

    lang.mTokenRegexStrings.push_back(std::make_pair("\\*\\*[^*]+\\*\\*", TextEditor::PaletteIndex::Keyword));

    lang.mTokenRegexStrings.push_back(std::make_pair("\\*[^*]+\\*", TextEditor::PaletteIndex::KnownIdentifier));

    lang.mTokenRegexStrings.push_back(std::make_pair("^[ \\t]*#+", TextEditor::PaletteIndex::Preprocessor));

    lang.mTokenRegexStrings.push_back(std::make_pair("^[ \\t]*>", TextEditor::PaletteIndex::Comment));

    ApplyNoCommentParsing(lang);
    lang.mCaseSensitive = true;

    return lang;
}

void LoadFile(const std::string& filepath, TextEditor& editor, AppState& state) {
    std::ifstream file(filepath);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        editor.SetText(buffer.str());
        state.currentFilePath = filepath;
    }
}

void SaveFile(const std::string& filepath, TextEditor& editor, AppState& state) {
    std::ofstream file(filepath);
    if (file.is_open()) {
        file << editor.GetText();
        state.currentFilePath = filepath;
    }
}

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    AppState state;
    std::vector<ImFont*> editorFonts;
    int currentFontIndex = kDefaultFontIndex;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGuiIO& io = ImGui::GetIO();
    LoadEditorFonts(io, editorFonts);

    TextEditor::LanguageDefinition markdownDef = GetMarkdownDefinition();

    TextEditor editor;
    editor.SetPalette(TextEditor::GetDarkPalette());
    editor.SetText("_Start *typing* or open a file..._");

    TextEditor::LanguageDefinition plainTextDef;
    plainTextDef.mName = "Plain Text";
    ApplyNoCommentParsing(plainTextDef);
    editor.SetLanguageDefinition(plainTextDef);

    SetMarkdownMode(state.enableMarkdown, editor, markdownDef, plainTextDef);

    ImVec4 clear_color = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (state.currentFilePath != state.lastFilePath) {
            std::string title = state.currentFilePath.empty() ? "Fast Notepad - Untitled"
                                                              : "Fast Notepad - " + state.currentFilePath;
            glfwSetWindowTitle(window, title.c_str());
            state.lastFilePath = state.currentFilePath;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false)) state.triggerOpen = true;
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
            if (io.KeyShift) {
                state.triggerSaveAs = true;
            } else {
                state.triggerSave = true;
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
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_0, false)) currentFontIndex = kDefaultFontIndex;

        ClampFontIndex(currentFontIndex, (int)editorFonts.size());
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    editor.SetText("");
                    state.currentFilePath = "";
                }

                if (ImGui::MenuItem("Open", "Ctrl+O")) {
                    state.triggerOpen = true;
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    state.triggerSave = true;
                }
                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {
                    state.triggerSaveAs = true;
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
                bool prevMarkdownState = state.enableMarkdown;
                ImGui::MenuItem("Markdown Highlighting", nullptr, &state.enableMarkdown);

                if (state.enableMarkdown != prevMarkdownState) {
                    SetMarkdownMode(state.enableMarkdown, editor, markdownDef, plainTextDef);
                }

                ImGui::Separator();

                bool prevThemeState = state.isDarkTheme;
                ImGui::MenuItem("Dark Theme", nullptr, &state.isDarkTheme);
                if (state.isDarkTheme != prevThemeState) {
                    ApplyTheme(state.isDarkTheme, editor, clear_color);
                }

                if (ImGui::MenuItem("Zoom In", "Ctrl++")) {
                    currentFontIndex++;
                }
                if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
                    currentFontIndex--;
                }
                if (ImGui::MenuItem("Reset Zoom", "Ctrl+0")) {
                    currentFontIndex = kDefaultFontIndex;
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

        if (state.triggerOpen) {
            auto f = pfd::open_file("Choose text file", ".", {"Text Files", "*.txt *.md *.cpp *.h", "All Files", "*"});
            if (!f.result().empty()) {
                LoadFile(f.result()[0], editor, state);
            }
            state.triggerOpen = false;
        }
        if (state.triggerSave) {
            if (state.currentFilePath.empty()) {
                state.triggerSaveAs = true;
            } else {
                SaveFile(state.currentFilePath, editor, state);
            }
            state.triggerSave = false;
        }
        if (state.triggerSaveAs) {
            auto f = pfd::save_file("Save file", ".", {"Text Files", "*.txt *.md"});
            if (!f.result().empty()) {
                SaveFile(f.result(), editor, state);
            }
            state.triggerSaveAs = false;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}