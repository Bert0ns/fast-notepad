import os

files = {}

files["src/Platform.h"] = """\
#pragma once
#include <string>
#include <vector>

struct GLFWwindow;

class Platform {
public:
    static std::vector<std::string> GetIconSearchPaths();
    static bool SetWindowIcon(GLFWwindow* window);
    static const char* FindMonospaceFontPath();
};
"""

files["src/Platform.cpp"] = """\
#include "Platform.h"
#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "stb_image.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include "resource.h"
#endif

std::vector<std::string> Platform::GetIconSearchPaths() {
  std::vector<std::string> paths = {"icon.png", "../icon.png", "../assets/icon.png"};
  std::error_code ec;
  auto cwd = std::filesystem::current_path(ec);
  if (!ec) {
    paths.push_back((cwd / "icon.png").string());
    paths.push_back((cwd.parent_path() / "icon.png").string());
  }
  return paths;
}

bool Platform::SetWindowIcon(GLFWwindow* window) {
#ifdef _WIN32
  HWND hwnd = glfwGetWin32Window(window);
  if (!hwnd) return false;
  HICON icon = static_cast<HICON>(LoadImageW(GetModuleHandleW(nullptr),
                                             MAKEINTRESOURCEW(IDI_ICON1),
                                             IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
  if (!icon) return false;
  SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
  SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
  return true;
#else
  int width = 0, height = 0, channels = 0;
  for (const auto& path : GetIconSearchPaths()) {
    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) continue;
    GLFWimage image;
    image.width = width;
    image.height = height;
    image.pixels = pixels;
    glfwSetWindowIcon(window, 1, &image);
    stbi_image_free(pixels);
    return true;
  }
  return false;
#endif
}

const char* Platform::FindMonospaceFontPath() {
  std::ifstream f_win("C:\\\\Windows\\\\Fonts\\\\consola.ttf");
  if (f_win.good()) return "C:\\\\Windows\\\\Fonts\\\\consola.ttf";
  std::ifstream f_lin("/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf");
  if (f_lin.good()) return "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";
  return nullptr;
}
"""

files["src/ThemeManager.h"] = """\
#pragma once
#include "TextEditor.h"
#include <imgui.h>

class ThemeManager {
public:
    ThemeManager();
    void ApplyTheme(bool isDarkTheme, TextEditor& editor);
    void ApplyMarkdownMode(bool enableMarkdown, TextEditor& editor);
    
    ImVec4 GetClearColor() const { return m_clearColor; }
    
private:
    void ApplyNoCommentParsing(TextEditor::LanguageDefinition& lang);
    TextEditor::LanguageDefinition GetMarkdownDefinition();

    TextEditor::LanguageDefinition m_markdownDef;
    TextEditor::LanguageDefinition m_plainTextDef;
    ImVec4 m_clearColor;
};
"""

files["src/ThemeManager.cpp"] = """\
#include "ThemeManager.h"

ThemeManager::ThemeManager() {
    m_markdownDef = GetMarkdownDefinition();
    
    m_plainTextDef.mName = "Plain Text";
    ApplyNoCommentParsing(m_plainTextDef);
}

void ThemeManager::ApplyNoCommentParsing(TextEditor::LanguageDefinition& lang) {
    lang.mCommentStart = "\\x01";
    lang.mCommentEnd = "\\x01";
    lang.mSingleLineComment = "\\x01";
    lang.mPreprocChar = 0;
    lang.mAutoIndentation = false;
}

TextEditor::LanguageDefinition ThemeManager::GetMarkdownDefinition() {
    TextEditor::LanguageDefinition lang;
    lang.mName = "Markdown";
    lang.mTokenRegexStrings.push_back(std::make_pair("`[^`]+`", TextEditor::PaletteIndex::String));
    lang.mTokenRegexStrings.push_back(std::make_pair("\\\\*\\\\*[^*]+\\\\*\\\\*", TextEditor::PaletteIndex::Keyword));
    lang.mTokenRegexStrings.push_back(std::make_pair("\\\\*[^*]+\\\\*", TextEditor::PaletteIndex::KnownIdentifier));
    lang.mTokenRegexStrings.push_back(std::make_pair("^[ \\\\t]*#+", TextEditor::PaletteIndex::Preprocessor));
    lang.mTokenRegexStrings.push_back(std::make_pair("^[ \\\\t]*>", TextEditor::PaletteIndex::Comment));
    ApplyNoCommentParsing(lang);
    lang.mCaseSensitive = true;
    return lang;
}

void ThemeManager::ApplyTheme(bool isDarkTheme, TextEditor& editor) {
    if (isDarkTheme) {
        ImGui::StyleColorsDark();
        editor.SetPalette(TextEditor::GetDarkPalette());
        m_clearColor = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    } else {
        ImGui::StyleColorsLight();
        editor.SetPalette(TextEditor::GetLightPalette());
        m_clearColor = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    }
}

void ThemeManager::ApplyMarkdownMode(bool enableMarkdown, TextEditor& editor) {
    editor.SetLanguageDefinition(enableMarkdown ? m_markdownDef : m_plainTextDef);
}
"""

files["src/SessionManager.h"] = """\
#pragma once
#include <string>

class SessionManager {
public:
    static void SaveSessionState(const std::string& currentFilePath, const std::string& content);
    static bool LoadSessionState(std::string& currentFilePath, std::string& content);
};
"""

files["src/SessionManager.cpp"] = """\
#include "SessionManager.h"
#include <filesystem>
#include <fstream>
#include <sstream>

void SessionManager::SaveSessionState(const std::string& currentFilePath, const std::string& content) {
    std::error_code ec;
    auto path = std::filesystem::temp_directory_path(ec) / "fast-notepad-autosave.txt";
    if (ec) path = "fast-notepad-autosave.txt";

    std::ofstream file(path, std::ios::binary);
    if (file.is_open()) {
        file << currentFilePath << '\\n';
        file << content;
    }
}

bool SessionManager::LoadSessionState(std::string& currentFilePath, std::string& content) {
    std::error_code ec;
    auto path = std::filesystem::temp_directory_path(ec) / "fast-notepad-autosave.txt";
    if (ec) path = "fast-notepad-autosave.txt";

    std::ifstream file(path, std::ios::binary);
    if (file.is_open()) {
        std::string filepath;
        std::getline(file, filepath);
        if (!filepath.empty() && filepath.back() == '\\r') {
            filepath.pop_back();
        }
        currentFilePath = filepath;
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();

        bool isEmpty = true;
        for (char c : content) {
            if (!std::isspace(static_cast<unsigned char>(c))) {
                isEmpty = false;
                break;
            }
        }

        if (currentFilePath.empty() && isEmpty) {
            return false;
        }

        return true;
    }
    return false;
}
"""

files["src/FileHandler.h"] = """\
#pragma once
#include <string>
#include "TextEditor.h"

class FileHandler {
public:
    void HandleDialogs(std::string& currentFilePath, TextEditor& editor, bool& triggerOpen, bool& triggerSave, bool& triggerSaveAs);
    void LoadFile(const std::string& filepath, TextEditor& editor, std::string& currentFilePath);
    void SaveFile(const std::string& filepath, TextEditor& editor, std::string& currentFilePath);
};
"""

files["src/FileHandler.cpp"] = """\
#include "FileHandler.h"
#include <fstream>
#include <sstream>
#include "portable-file-dialogs.h"

void FileHandler::LoadFile(const std::string& filepath, TextEditor& editor, std::string& currentFilePath) {
    std::ifstream file(filepath);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        editor.SetText(buffer.str());
        currentFilePath = filepath;
    }
}

void FileHandler::SaveFile(const std::string& filepath, TextEditor& editor, std::string& currentFilePath) {
    std::ofstream file(filepath);
    if (file.is_open()) {
        file << editor.GetText();
        currentFilePath = filepath;
    }
}

void FileHandler::HandleDialogs(std::string& currentFilePath, TextEditor& editor, bool& triggerOpen, bool& triggerSave, bool& triggerSaveAs) {
    if (triggerOpen) {
        auto f = pfd::open_file("Choose text file", ".", {"Text Files", "*.txt *.md *.cpp *.h", "All Files", "*"});
        if (!f.result().empty()) {
            LoadFile(f.result()[0], editor, currentFilePath);
        }
        triggerOpen = false;
    }
    if (triggerSave) {
        if (currentFilePath.empty()) {
            triggerSaveAs = true;
        } else {
            SaveFile(currentFilePath, editor, currentFilePath);
        }
        triggerSave = false;
    }
    if (triggerSaveAs) {
        auto f = pfd::save_file("Save file", ".", {"Text Files", "*.txt *.md"});
        if (!f.result().empty()) {
            SaveFile(f.result(), editor, currentFilePath);
        }
        triggerSaveAs = false;
    }
}
"""

files["src/FindPanel.h"] = """\
#pragma once
#include "TextEditor.h"
#include <imgui.h>
#include <string>
#include <array>

class FindPanel {
public:
    void Render(TextEditor& editor, ImGuiViewport* viewport);
    void Open(TextEditor& editor);
    void Close();
    bool IsOpen() const { return m_show; }

private:
    void UpdateBufferFromSelection(TextEditor& editor);
    bool FindNextMatch(TextEditor& editor, const std::string& query, bool wrap, bool& wrapped);
    
    int Utf8CharLength(unsigned char c);
    int ByteIndexToColumn(const std::string& line, int byteIndex, int tabSize);
    int ColumnToByteIndex(const std::string& line, int column, int tabSize);

    bool m_show = false;
    bool m_focusInput = false;
    bool m_wrapSearch = true;
    bool m_findFailed = false;
    bool m_findWrapped = false;
    std::array<char, 256> m_findBuffer = {0};
};
"""

files["src/FindPanel.cpp"] = """\
#include "FindPanel.h"

void FindPanel::Open(TextEditor& editor) {
    UpdateBufferFromSelection(editor);
    m_show = true;
    m_focusInput = true;
}

void FindPanel::Close() {
    m_show = false;
}

void FindPanel::UpdateBufferFromSelection(TextEditor& editor) {
    if (!editor.HasSelection()) return;
    std::string selected = editor.GetSelectedText();
    auto newlinePos = selected.find('\\n');
    if (newlinePos != std::string::npos) selected = selected.substr(0, newlinePos);
    if (selected.empty()) return;
    std::snprintf(m_findBuffer.data(), m_findBuffer.size(), "%s", selected.c_str());
}

int FindPanel::Utf8CharLength(unsigned char c) {
    if ((c & 0xFE) == 0xFC) return 6;
    if ((c & 0xFC) == 0xF8) return 5;
    if ((c & 0xF8) == 0xF0) return 4;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xE0) == 0xC0) return 2;
    return 1;
}

int FindPanel::ByteIndexToColumn(const std::string& line, int byteIndex, int tabSize) {
    int column = 0;
    int i = 0;
    const int limit = std::min<int>(byteIndex, static_cast<int>(line.size()));
    while (i < limit) {
        unsigned char c = static_cast<unsigned char>(line[i]);
        if (c == '\\t') {
            column = (column / tabSize) * tabSize + tabSize;
            ++i;
        } else {
            ++column;
            i += Utf8CharLength(c);
        }
    }
    return column;
}

int FindPanel::ColumnToByteIndex(const std::string& line, int column, int tabSize) {
    int currentColumn = 0;
    int i = 0;
    while (i < static_cast<int>(line.size()) && currentColumn < column) {
        unsigned char c = static_cast<unsigned char>(line[i]);
        if (c == '\\t') {
            int nextColumn = (currentColumn / tabSize) * tabSize + tabSize;
            if (nextColumn > column) break;
            currentColumn = nextColumn;
            ++i;
        } else {
            ++currentColumn;
            i += Utf8CharLength(c);
        }
    }
    return i;
}

bool FindPanel::FindNextMatch(TextEditor& editor, const std::string& query, bool wrap, bool& wrapped) {
    wrapped = false;
    if (query.empty()) return false;
    const auto lines = editor.GetTextLines();
    if (lines.empty()) return false;

    const auto cursor = editor.GetCursorPosition();
    int startLine = std::min<int>(cursor.mLine, static_cast<int>(lines.size() - 1));
    int startByte = ColumnToByteIndex(lines[startLine], cursor.mColumn, editor.GetTabSize());

    for (int line = startLine; line < static_cast<int>(lines.size()); ++line) {
        const auto& text = lines[line];
        const size_t from = (line == startLine) ? static_cast<size_t>(startByte) : 0u;
        const size_t pos = text.find(query, from);
        if (pos != std::string::npos) {
            int startCol = ByteIndexToColumn(text, static_cast<int>(pos), editor.GetTabSize());
            int endCol = ByteIndexToColumn(text, static_cast<int>(pos + query.size()), editor.GetTabSize());
            editor.SetSelection({line, startCol}, {line, endCol});
            editor.SetCursorPosition({line, endCol});
            return true;
        }
    }

    if (!wrap) return false;

    for (int line = 0; line <= startLine; ++line) {
        const auto& text = lines[line];
        const size_t limit = (line == startLine) ? static_cast<size_t>(startByte) : text.size();
        size_t pos = text.find(query, 0u);
        if (pos != std::string::npos && pos < limit) {
            int startCol = ByteIndexToColumn(text, static_cast<int>(pos), editor.GetTabSize());
            int endCol = ByteIndexToColumn(text, static_cast<int>(pos + query.size()), editor.GetTabSize());
            editor.SetSelection({line, startCol}, {line, endCol});
            editor.SetCursorPosition({line, endCol});
            wrapped = true;
            return true;
        }
    }
    return false;
}

void FindPanel::Render(TextEditor& editor, ImGuiViewport* viewport) {
    if (!m_show) return;

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 340.0f, viewport->WorkPos.y + 12.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(320.0f, 0.0f), ImGuiCond_Always);
    ImGuiWindowFlags findFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    
    if (ImGui::Begin("Find", &m_show, findFlags)) {
        if (m_focusInput) {
            ImGui::SetKeyboardFocusHere();
            m_focusInput = false;
        }

        ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
        bool submit = ImGui::InputText("##FindInput", m_findBuffer.data(), m_findBuffer.size(), inputFlags);
        ImGui::SameLine();
        if (ImGui::Button("Find Next")) submit = true;

        ImGui::SameLine();
        ImGui::Checkbox("Wrap", &m_wrapSearch);

        if (submit) {
            std::string query(m_findBuffer.data());
            m_findFailed = !FindNextMatch(editor, query, m_wrapSearch, m_findWrapped);
            if (!m_findFailed) m_show = true;
        }

        if (m_findFailed) {
            ImGui::TextUnformatted("No matches found.");
        } else if (m_findWrapped) {
            ImGui::TextUnformatted("Wrapped to start.");
        }
    }
    ImGui::End();
}
"""

files["src/NotepadApp.h"] = """\
#pragma once
#include <string>
#include <vector>
#include "TextEditor.h"
#include "ThemeManager.h"
#include "FileHandler.h"
#include "FindPanel.h"

struct GLFWwindow;

class NotepadApp {
public:
    NotepadApp();
    ~NotepadApp();

    bool Init();
    int Run();

private:
    void LoadFonts();
    void HandleShortcuts();
    void RenderMenuBar();
    void RenderEditor();
    void UpdateWindowTitle();
    void SelectAllText();

    GLFWwindow* m_window = nullptr;
    
    TextEditor m_editor;
    ThemeManager m_themeManager;
    FileHandler m_fileHandler;
    FindPanel m_findPanel;

    std::string m_currentFilePath;
    std::string m_lastFilePath = "UNINITIALIZED";
    
    bool m_triggerOpen = false;
    bool m_triggerSaveAs = false;
    bool m_triggerSave = false;
    
    bool m_enableMarkdown = true;
    bool m_isDarkTheme = true;
    bool m_focusEditorOnStart = true;

    std::vector<struct ImFont*> m_editorFonts;
    struct ImFont* m_menuFont = nullptr;
    int m_currentFontIndex = 3; // kDefaultFontIndex
};
"""

files["src/NotepadApp.cpp"] = """\
#include "NotepadApp.h"
#include "Platform.h"
#include "SessionManager.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

constexpr int kDefaultFontIndex = 3;
constexpr int kFontMinSize = 14;
constexpr int kFontMaxSize = 32;
constexpr int kFontStep = 2;
constexpr int kFontSlots = ((kFontMaxSize - kFontMinSize) / kFontStep) + 1;
constexpr float kMenuFontSize = 20.0f;
constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 600;
constexpr const char* kWindowTitle = "Fast Notepad";
constexpr float kMenuBarPaddingX = 12.0f;
constexpr float kMenuBarPaddingY = 12.0f;

NotepadApp::NotepadApp() {}

NotepadApp::~NotepadApp() {
    if (m_window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
}

bool NotepadApp::Init() {
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, NULL, NULL);
    if (!m_window) {
        glfwTerminate();
        return false;
    }

    Platform::SetWindowIcon(m_window);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    LoadFonts();

    m_themeManager.ApplyTheme(m_isDarkTheme, m_editor);
    m_themeManager.ApplyMarkdownMode(m_enableMarkdown, m_editor);

    std::string content;
    if (SessionManager::LoadSessionState(m_currentFilePath, content)) {
        m_editor.SetText(content);
    } else {
        m_editor.SetText("Start *typing* or open a file...");
        SelectAllText();
    }

    return true;
}

void NotepadApp::LoadFonts() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    
    const char* fontPath = Platform::FindMonospaceFontPath();
    if (fontPath) {
        for (int size = kFontMinSize; size <= kFontMaxSize; size += kFontStep) {
            m_editorFonts.push_back(io.Fonts->AddFontFromFileTTF(fontPath, size));
        }
    } else {
        for (int i = 0; i < kFontSlots; ++i) {
            m_editorFonts.push_back(nullptr);
        }
    }

    ImFontConfig config;
    config.SizePixels = kMenuFontSize;
    m_menuFont = io.Fonts->AddFontDefault(&config);
}

void NotepadApp::SelectAllText() {
    const auto lines = m_editor.GetTextLines();
    if (lines.empty()) {
        m_editor.SetCursorPosition({0, 0});
        return;
    }
    const int lastLine = static_cast<int>(lines.size() - 1);
    
    int endCol = 0;
    const std::string& line = lines[lastLine];
    int currentColumn = 0;
    int i = 0;
    int tabSize = m_editor.GetTabSize();
    while (i < static_cast<int>(line.size())) {
        unsigned char c = static_cast<unsigned char>(line[i]);
        if (c == '\\t') {
            currentColumn = (currentColumn / tabSize) * tabSize + tabSize;
            ++i;
        } else {
            ++currentColumn;
            int len = 1;
            if ((c & 0xFE) == 0xFC) len = 6;
            else if ((c & 0xFC) == 0xF8) len = 5;
            else if ((c & 0xF8) == 0xF0) len = 4;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xE0) == 0xC0) len = 2;
            i += len;
        }
    }
    endCol = currentColumn;

    m_editor.SetSelection({0, 0}, {lastLine, endCol});
    m_editor.SetCursorPosition({lastLine, endCol});
}

void NotepadApp::HandleShortcuts() {
    ImGuiIO& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false)) m_triggerOpen = true;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        if (io.KeyShift) m_triggerSaveAs = true;
        else m_triggerSave = true;
    }
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false)) {
        m_findPanel.Open(m_editor);
    }
    if (io.KeyCtrl && io.MouseWheel != 0.0f) {
        if (io.MouseWheel > 0) m_currentFontIndex++;
        else m_currentFontIndex--;
    }
    if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Equal, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd, false) || ImGui::IsKeyPressed(ImGuiKey_RightBracket, false))) m_currentFontIndex++;
    if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Minus, false) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract, false))) m_currentFontIndex--;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_0, false)) m_currentFontIndex = kDefaultFontIndex;

    if (m_findPanel.IsOpen() && ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
        m_findPanel.Close();
    }
}

void NotepadApp::RenderMenuBar() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(kMenuBarPaddingX, kMenuBarPaddingY));
    if (ImGui::BeginMainMenuBar()) {
        if (m_menuFont) ImGui::PushFont(m_menuFont);
        
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                m_editor.SetText("");
                m_currentFilePath = "";
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) m_triggerOpen = true;
            if (ImGui::MenuItem("Save", "Ctrl+S")) m_triggerSave = true;
            if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) m_triggerSaveAs = true;
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) glfwSetWindowShouldClose(m_window, true);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", nullptr, m_editor.CanUndo())) m_editor.Undo();
            if (ImGui::MenuItem("Redo", "Ctrl+Y", nullptr, m_editor.CanRedo())) m_editor.Redo();
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X", nullptr, m_editor.HasSelection())) m_editor.Cut();
            if (ImGui::MenuItem("Copy", "Ctrl+C", nullptr, m_editor.HasSelection())) m_editor.Copy();
            if (ImGui::MenuItem("Paste", "Ctrl+V")) m_editor.Paste();
            ImGui::Separator();
            if (ImGui::MenuItem("Find", "Ctrl+F")) m_findPanel.Open(m_editor);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            bool prevMarkdown = m_enableMarkdown;
            ImGui::MenuItem("Markdown Highlighting", nullptr, &m_enableMarkdown);
            if (m_enableMarkdown != prevMarkdown) {
                m_themeManager.ApplyMarkdownMode(m_enableMarkdown, m_editor);
            }

            ImGui::Separator();

            bool prevTheme = m_isDarkTheme;
            ImGui::MenuItem("Dark Theme", nullptr, &m_isDarkTheme);
            if (m_isDarkTheme != prevTheme) {
                m_themeManager.ApplyTheme(m_isDarkTheme, m_editor);
            }

            if (ImGui::MenuItem("Zoom In", "Ctrl++")) m_currentFontIndex++;
            if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) m_currentFontIndex--;
            if (ImGui::MenuItem("Reset Zoom", "Ctrl+0")) m_currentFontIndex = kDefaultFontIndex;

            ImGui::EndMenu();
        }
        if (m_menuFont) ImGui::PopFont();
        ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleVar();
}

void NotepadApp::RenderEditor() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("MainWorkspace", nullptr, windowFlags);
    
    if (m_focusEditorOnStart) {
        ImGui::SetKeyboardFocusHere();
        m_focusEditorOnStart = false;
    }

    if (m_currentFontIndex >= 0 && m_currentFontIndex < (int)m_editorFonts.size() && m_editorFonts[m_currentFontIndex]) {
        ImGui::PushFont(m_editorFonts[m_currentFontIndex]);
    }

    m_editor.Render("TextEditor");

    if (m_currentFontIndex >= 0 && m_currentFontIndex < (int)m_editorFonts.size() && m_editorFonts[m_currentFontIndex]) {
        ImGui::PopFont();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void NotepadApp::UpdateWindowTitle() {
    if (m_currentFilePath != m_lastFilePath) {
        std::string title = m_currentFilePath.empty() ? "Fast Notepad - Untitled" : "Fast Notepad - " + m_currentFilePath;
        glfwSetWindowTitle(m_window, title.c_str());
        m_lastFilePath = m_currentFilePath;
    }
}

int NotepadApp::Run() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        UpdateWindowTitle();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        HandleShortcuts();
        if (m_currentFontIndex < 0) m_currentFontIndex = 0;
        if (m_currentFontIndex >= (int)m_editorFonts.size()) m_currentFontIndex = (int)m_editorFonts.size() - 1;

        RenderMenuBar();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        RenderEditor();
        m_findPanel.Render(m_editor, viewport);

        ImGui::Render();
        ImVec4 clearColor = m_themeManager.GetClearColor();
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_window);

        m_fileHandler.HandleDialogs(m_currentFilePath, m_editor, m_triggerOpen, m_triggerSave, m_triggerSaveAs);
    }

    SessionManager::SaveSessionState(m_currentFilePath, m_editor.GetText());
    return 0;
}
"""

files["src/main.cpp"] = """\
#include "NotepadApp.h"

int main() {
    NotepadApp app;
    if (!app.Init()) {
        return -1;
    }
    return app.Run();
}
"""

files["CMakeLists.txt"] = """\
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
"""

for filepath, content in files.items():
    os.makedirs(os.path.dirname(filepath) or '.', exist_ok=True)
    with open(filepath, 'w') as f:
        f.write(content)

print("Refactoring complete.")
