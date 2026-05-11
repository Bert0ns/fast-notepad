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

// --- CUSTOM MARKDOWN DEFINITION (PRIORITY FIXED) ---
TextEditor::LanguageDefinition GetMarkdownDefinition() {
    TextEditor::LanguageDefinition lang;
    lang.mName = "Markdown";

    // 1. INLINE CODE (Highest Priority)
    // Matches `code`. We want this first so asterisks inside backticks don't trigger bold/italic.
    lang.mTokenRegexStrings.push_back(std::make_pair("`[^`]+`", TextEditor::PaletteIndex::String));

    // 2. BOLD
    // Must come BEFORE italic, otherwise the single '*' rule will eat the first half of the '**'
    lang.mTokenRegexStrings.push_back(std::make_pair("\\*\\*[^*]+\\*\\*", TextEditor::PaletteIndex::Keyword));

    // 3. ITALIC
    lang.mTokenRegexStrings.push_back(std::make_pair("\\*[^*]+\\*", TextEditor::PaletteIndex::KnownIdentifier));

    // 4. HEADERS (Modified)
    // Instead of eating the whole line, we ONLY color the '#' symbols.
    // This allows text immediately following the header to still parse bold/italics!
    lang.mTokenRegexStrings.push_back(std::make_pair("^[ \\t]*#+", TextEditor::PaletteIndex::Preprocessor));

    // 5. BLOCKQUOTES (Modified)
    // Same logic: only color the '>' symbol so the quote text can still be formatted.
    lang.mTokenRegexStrings.push_back(std::make_pair("^[ \\t]*>", TextEditor::PaletteIndex::Comment));

    // Disable standard C++ parsing rules
    lang.mCommentStart = "";
    lang.mCommentEnd = "";
    lang.mSingleLineComment = "";
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
    std::vector<ImFont*> editorFonts;  // NEW: Store multiple crisp font sizes
    int currentFontIndex = 2;          // NEW: Default to index 2 (18px)

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // --- NEW: SMART FONT LOADING ---
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();  // Load default tiny font for the Menu Bar

    // Try to find a standard monospace font based on the OS
    const char* fontPath = nullptr;
    std::ifstream f_win("C:\\Windows\\Fonts\\consola.ttf");
    std::ifstream f_lin("/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf");
    if (f_win.good())
        fontPath = "C:\\Windows\\Fonts\\consola.ttf";
    else if (f_lin.good())
        fontPath = "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";

    // Load the font at 10 different, pixel-perfect sizes
    if (fontPath) {
        for (int size = 14; size <= 32; size += 2) {
            editorFonts.push_back(io.Fonts->AddFontFromFileTTF(fontPath, size));
        }
    } else {
        // Fallback if font isn't found
        for (int i = 0; i < 10; ++i) editorFonts.push_back(nullptr);
    }

    TextEditor editor;
    editor.SetPalette(TextEditor::GetDarkPalette());
    editor.SetText("_Start *typing* or open a file..._");

    // We start with Plain Text (an empty definition)
    TextEditor::LanguageDefinition plainTextDef;
    plainTextDef.mName = "Plain Text";
    editor.SetLanguageDefinition(plainTextDef);

    // Create a persistent Markdown definition so toggling doesn't pass temporaries
    TextEditor::LanguageDefinition markdownDef = GetMarkdownDefinition();

    // State flags for file operations
    bool triggerOpen = false;
    bool triggerSaveAs = false;
    bool triggerSave = false;

    // markdown toggle state
    bool enableMarkdown = false;

    std::string lastFilePath = "UNINITIALIZED";  // Force an update on the first frame

    bool isDarkTheme = true;
    ImVec4 clear_color = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);  // Default to dark grey

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // --- ONLY UPDATE TITLE IF THE FILE CHANGED ---
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
        // FONT SCALING SHORTCUTS
        if (io.KeyCtrl && io.MouseWheel != 0.0f) {
            if (io.MouseWheel > 0)
                currentFontIndex++;
            else
                currentFontIndex--;
        }
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Equal, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd, false) || ImGui::IsKeyPressed(ImGuiKey_RightBracket, false))) currentFontIndex++;
        if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Minus, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract, false))) currentFontIndex--;
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_0, false)) currentFontIndex = 2;  // Reset

        // Clamp the zoom index safely within our loaded font sizes
        if (currentFontIndex < 0) currentFontIndex = 0;
        if (currentFontIndex >= (int)editorFonts.size()) currentFontIndex = (int)editorFonts.size() - 1;

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
            if (ImGui::BeginMenu("View")) {
                // --- TOGGLE MARKDOWN LOGIC ---
                bool prevMarkdownState = enableMarkdown;
                // Passing &enableMarkdown adds a checkmark UI to the menu item!
                ImGui::MenuItem("Markdown Highlighting", nullptr, &enableMarkdown);

                // If the user clicked it, swap the language definition
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
                        clear_color = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);  // Clean off-white
                    }
                }

                // --- NEW ZOOM CONTROLS ---
                if (ImGui::MenuItem("Zoom In", "Ctrl++")) {
                    currentFontIndex++;
                }
                if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
                    currentFontIndex--;
                }
                if (ImGui::MenuItem("Reset Zoom", "Ctrl+0")) {
                    currentFontIndex = 2;
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
        // Push the crisp font ONLY for the editor!
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