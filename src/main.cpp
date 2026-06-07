#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>

#include "resource.h"
#endif

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "TextEditor.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "portable-file-dialogs.h"
#include "stb_image.h"

namespace {
constexpr int kDefaultFontIndex = 3;
constexpr int kFontMinSize = 14;
constexpr int kFontMaxSize = 32;
constexpr int kFontStep = 2;
constexpr int kFontSlots = ((kFontMaxSize - kFontMinSize) / kFontStep) + 1;
constexpr float kMenuFontSize = 20.0f;
constexpr const char* kNoCommentSentinel = "\x01";
constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 600;
constexpr const char* kWindowTitle = "Fast Notepad";
constexpr float kMenuBarPaddingX = 12.0f;
constexpr float kMenuBarPaddingY = 12.0f;

struct AppState {
  std::string currentFilePath;
  std::string lastFilePath = "UNINITIALIZED";
  bool triggerOpen = false;
  bool triggerSaveAs = false;
  bool triggerSave = false;
  bool enableMarkdown = true;
  bool isDarkTheme = true;
  bool showFindPanel = false;
  bool focusFindInput = false;
  bool findWrapped = false;
  bool findFailed = false;
  std::array<char, 256> findBuffer = {0};
  bool wrapSearch = true;
  bool focusEditorOnStart = true;
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

ImFont* LoadMenuFont(ImGuiIO& io) {
  ImFontConfig config;
  config.SizePixels = kMenuFontSize;
  return io.Fonts->AddFontDefault(&config);
}

void SetMarkdownMode(bool enableMarkdown, TextEditor& editor,
                     const TextEditor::LanguageDefinition& markdownDef,
                     const TextEditor::LanguageDefinition& plainTextDef) {
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

void UpdateFindBufferFromSelection(AppState& state, TextEditor& editor) {
  if (!editor.HasSelection()) return;

  std::string selected = editor.GetSelectedText();
  auto newlinePos = selected.find('\n');
  if (newlinePos != std::string::npos) {
    selected = selected.substr(0, newlinePos);
  }

  if (selected.empty()) return;
  std::snprintf(state.findBuffer.data(), state.findBuffer.size(), "%s",
                selected.c_str());
}

void OpenFindPanel(AppState& state, TextEditor& editor) {
  UpdateFindBufferFromSelection(state, editor);
  state.showFindPanel = true;
  state.focusFindInput = true;
}

int Utf8CharLength(unsigned char c) {
  if ((c & 0xFE) == 0xFC) return 6;
  if ((c & 0xFC) == 0xF8) return 5;
  if ((c & 0xF8) == 0xF0) return 4;
  if ((c & 0xF0) == 0xE0) return 3;
  if ((c & 0xE0) == 0xC0) return 2;
  return 1;
}

int ByteIndexToColumn(const std::string& line, int byteIndex, int tabSize) {
  int column = 0;
  int i = 0;
  const int limit = std::min<int>(byteIndex, static_cast<int>(line.size()));
  while (i < limit) {
    unsigned char c = static_cast<unsigned char>(line[i]);
    if (c == '\t') {
      column = (column / tabSize) * tabSize + tabSize;
      ++i;
    } else {
      ++column;
      i += Utf8CharLength(c);
    }
  }
  return column;
}

int ColumnToByteIndex(const std::string& line, int column, int tabSize) {
  int currentColumn = 0;
  int i = 0;
  while (i < static_cast<int>(line.size()) && currentColumn < column) {
    unsigned char c = static_cast<unsigned char>(line[i]);
    if (c == '\t') {
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

bool FindNextMatch(TextEditor& editor, const std::string& query, bool wrap,
                   bool& wrapped) {
  wrapped = false;
  if (query.empty()) return false;

  const auto lines = editor.GetTextLines();
  if (lines.empty()) return false;

  const auto cursor = editor.GetCursorPosition();
  int startLine =
      std::min<int>(cursor.mLine, static_cast<int>(lines.size() - 1));
  int startByte =
      ColumnToByteIndex(lines[startLine], cursor.mColumn, editor.GetTabSize());

  for (int line = startLine; line < static_cast<int>(lines.size()); ++line) {
    const auto& text = lines[line];
    const size_t from =
        (line == startLine) ? static_cast<size_t>(startByte) : 0u;
    const size_t pos = text.find(query, from);
    if (pos != std::string::npos) {
      int startCol =
          ByteIndexToColumn(text, static_cast<int>(pos), editor.GetTabSize());
      int endCol = ByteIndexToColumn(text, static_cast<int>(pos + query.size()),
                                     editor.GetTabSize());
      editor.SetSelection({line, startCol}, {line, endCol});
      editor.SetCursorPosition({line, endCol});
      return true;
    }
  }

  if (!wrap) return false;

  for (int line = 0; line <= startLine; ++line) {
    const auto& text = lines[line];
    const size_t limit =
        (line == startLine) ? static_cast<size_t>(startByte) : text.size();
    size_t pos = text.find(query, 0u);
    if (pos != std::string::npos && pos < limit) {
      int startCol =
          ByteIndexToColumn(text, static_cast<int>(pos), editor.GetTabSize());
      int endCol = ByteIndexToColumn(text, static_cast<int>(pos + query.size()),
                                     editor.GetTabSize());
      editor.SetSelection({line, startCol}, {line, endCol});
      editor.SetCursorPosition({line, endCol});
      wrapped = true;
      return true;
    }
  }

  return false;
}

void UpdateWindowTitle(GLFWwindow* window, AppState& state) {
  if (state.currentFilePath != state.lastFilePath) {
    std::string title = state.currentFilePath.empty()
                            ? "Fast Notepad - Untitled"
                            : "Fast Notepad - " + state.currentFilePath;
    glfwSetWindowTitle(window, title.c_str());
    state.lastFilePath = state.currentFilePath;
  }
}

void HandleGlobalShortcuts(AppState& state, TextEditor& editor, ImGuiIO& io,
                           int& currentFontIndex) {
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false))
    state.triggerOpen = true;
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
    if (io.KeyShift) {
      state.triggerSaveAs = true;
    } else {
      state.triggerSave = true;
    }
  }
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false)) {
    OpenFindPanel(state, editor);
  }

  if (io.KeyCtrl && io.MouseWheel != 0.0f) {
    if (io.MouseWheel > 0)
      currentFontIndex++;
    else
      currentFontIndex--;
  }
  if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Equal, false) ||
                     ImGui::IsKeyPressed(ImGuiKey_KeypadAdd, false) ||
                     ImGui::IsKeyPressed(ImGuiKey_RightBracket, false)))
    currentFontIndex++;
  if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Minus, false) ||
                     ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract, false)))
    currentFontIndex--;
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_0, false))
    currentFontIndex = kDefaultFontIndex;

  if (state.showFindPanel && ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
    state.showFindPanel = false;
  }
}

void RenderMenuBar(AppState& state, TextEditor& editor,
                   const TextEditor::LanguageDefinition& markdownDef,
                   const TextEditor::LanguageDefinition& plainTextDef,
                   ImFont* menuFont, ImVec4& clearColor,
                   int& currentFontIndex) {
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                      ImVec2(kMenuBarPaddingX, kMenuBarPaddingY));
  if (ImGui::BeginMainMenuBar()) {
    if (menuFont) {
      ImGui::PushFont(menuFont);
    }
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
        glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
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
      ImGui::Separator();
      if (ImGui::MenuItem("Find", "Ctrl+F")) {
        OpenFindPanel(state, editor);
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      bool prevMarkdownState = state.enableMarkdown;
      ImGui::MenuItem("Markdown Highlighting", nullptr, &state.enableMarkdown);

      if (state.enableMarkdown != prevMarkdownState) {
        SetMarkdownMode(state.enableMarkdown, editor, markdownDef,
                        plainTextDef);
      }

      ImGui::Separator();

      bool prevThemeState = state.isDarkTheme;
      ImGui::MenuItem("Dark Theme", nullptr, &state.isDarkTheme);
      if (state.isDarkTheme != prevThemeState) {
        ApplyTheme(state.isDarkTheme, editor, clearColor);
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
    if (menuFont) {
      ImGui::PopFont();
    }
    ImGui::EndMainMenuBar();
  }
  ImGui::PopStyleVar();
}

void RenderEditorWindow(TextEditor& editor,
                        const std::vector<ImFont*>& editorFonts,
                        int currentFontIndex, ImGuiWindowFlags windowFlags,
                        bool& focusEditorOnce) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("MainWorkspace", nullptr, windowFlags);
  if (focusEditorOnce) {
    ImGui::SetKeyboardFocusHere();
    focusEditorOnce = false;
  }
  if (editorFonts[currentFontIndex] != nullptr) {
    ImGui::PushFont(editorFonts[currentFontIndex]);
  }

  editor.Render("TextEditor");

  if (editorFonts[currentFontIndex] != nullptr) {
    ImGui::PopFont();
  }
  ImGui::End();
  ImGui::PopStyleVar();
}

void SelectAllText(TextEditor& editor) {
  const auto lines = editor.GetTextLines();
  if (lines.empty()) {
    editor.SetCursorPosition({0, 0});
    return;
  }

  const int lastLine = static_cast<int>(lines.size() - 1);
  const int endCol = ByteIndexToColumn(lines[lastLine],
                                       static_cast<int>(lines[lastLine].size()),
                                       editor.GetTabSize());
  editor.SetSelection({0, 0}, {lastLine, endCol});
  editor.SetCursorPosition({lastLine, endCol});
}

void RenderFindPanel(AppState& state, TextEditor& editor,
                     ImGuiViewport* viewport) {
  if (!state.showFindPanel) return;

  ImGui::SetNextWindowPos(
      ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 340.0f,
             viewport->WorkPos.y + 12.0f),
      ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(320.0f, 0.0f), ImGuiCond_Always);
  ImGuiWindowFlags findFlags = ImGuiWindowFlags_NoCollapse |
                               ImGuiWindowFlags_NoSavedSettings |
                               ImGuiWindowFlags_AlwaysAutoResize;
  if (ImGui::Begin("Find", &state.showFindPanel, findFlags)) {
    if (state.focusFindInput) {
      ImGui::SetKeyboardFocusHere();
      state.focusFindInput = false;
    }

    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
    bool submit = ImGui::InputText("##FindInput", state.findBuffer.data(),
                                   state.findBuffer.size(), inputFlags);
    ImGui::SameLine();
    if (ImGui::Button("Find Next")) {
      submit = true;
    }

    ImGui::SameLine();
    ImGui::Checkbox("Wrap", &state.wrapSearch);

    if (submit) {
      std::string query(state.findBuffer.data());
      state.findFailed =
          !FindNextMatch(editor, query, state.wrapSearch, state.findWrapped);
      if (!state.findFailed) {
        state.showFindPanel = true;
      }
    }

    if (state.findFailed) {
      ImGui::TextUnformatted("No matches found.");
    } else if (state.findWrapped) {
      ImGui::TextUnformatted("Wrapped to start.");
    }
  }
  ImGui::End();
}

void LoadFile(const std::string& filepath, TextEditor& editor,
              AppState& state) {
  std::ifstream file(filepath);
  if (file.is_open()) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    editor.SetText(buffer.str());
    state.currentFilePath = filepath;
  }
}

void SaveFile(const std::string& filepath, TextEditor& editor,
              AppState& state) {
  std::ofstream file(filepath);
  if (file.is_open()) {
    file << editor.GetText();
    state.currentFilePath = filepath;
  }
}

void HandleFileDialogs(AppState& state, TextEditor& editor) {
  if (state.triggerOpen) {
    auto f = pfd::open_file(
        "Choose text file", ".",
        {"Text Files", "*.txt *.md *.cpp *.h", "All Files", "*"});
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

std::vector<std::string> GetIconSearchPaths() {
  std::vector<std::string> paths;
  paths.emplace_back("icon.png");
  paths.emplace_back("../icon.png");
  paths.emplace_back("../assets/icon.png");

  std::error_code ec;
  auto cwd = std::filesystem::current_path(ec);
  if (!ec) {
    paths.push_back((cwd / "icon.png").string());
    paths.push_back((cwd.parent_path() / "icon.png").string());
  }

  return paths;
}

bool SetWindowIcon(GLFWwindow* window) {
#ifdef _WIN32
  HWND hwnd = glfwGetWin32Window(window);
  if (!hwnd) {
    return false;
  }

  HICON icon = static_cast<HICON>(LoadImageW(GetModuleHandleW(nullptr),
                                             MAKEINTRESOURCEW(IDI_ICON1),
                                             IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
  if (!icon) {
    return false;
  }

  SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
  SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
  return true;
#else
  int width = 0;
  int height = 0;
  int channels = 0;

  for (const auto& path : GetIconSearchPaths()) {
    stbi_uc* pixels =
        stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) {
      continue;
    }

    GLFWimage image;
    image.width = width;
    image.height = height;
    image.pixels = pixels;
    glfwSetWindowIcon(window, 1, &image);
    stbi_image_free(pixels);
    return true;
  }

  std::cerr << "Failed to load icon.png: "
            << (stbi_failure_reason() ? stbi_failure_reason() : "unknown")
            << "\n";
  return false;
#endif
}
}  // namespace

TextEditor::LanguageDefinition GetMarkdownDefinition() {
  TextEditor::LanguageDefinition lang;
  lang.mName = "Markdown";

  lang.mTokenRegexStrings.push_back(
      std::make_pair("`[^`]+`", TextEditor::PaletteIndex::String));

  lang.mTokenRegexStrings.push_back(
      std::make_pair("\\*\\*[^*]+\\*\\*", TextEditor::PaletteIndex::Keyword));

  lang.mTokenRegexStrings.push_back(
      std::make_pair("\\*[^*]+\\*", TextEditor::PaletteIndex::KnownIdentifier));

  lang.mTokenRegexStrings.push_back(
      std::make_pair("^[ \\t]*#+", TextEditor::PaletteIndex::Preprocessor));

  lang.mTokenRegexStrings.push_back(
      std::make_pair("^[ \\t]*>", TextEditor::PaletteIndex::Comment));

  ApplyNoCommentParsing(lang);
  lang.mCaseSensitive = true;

  return lang;
}

int main() {
  if (!glfwInit()) return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  GLFWwindow* window =
      glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  SetWindowIcon(window);

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
  ImFont* menuFont = LoadMenuFont(io);

  TextEditor::LanguageDefinition markdownDef = GetMarkdownDefinition();

  TextEditor editor;
  editor.SetPalette(TextEditor::GetDarkPalette());
  editor.SetText("Start *typing* or open a file...");
  SelectAllText(editor);

  TextEditor::LanguageDefinition plainTextDef;
  plainTextDef.mName = "Plain Text";
  ApplyNoCommentParsing(plainTextDef);
  editor.SetLanguageDefinition(plainTextDef);

  SetMarkdownMode(state.enableMarkdown, editor, markdownDef, plainTextDef);

  ImVec4 clear_color = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    UpdateWindowTitle(window, state);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    HandleGlobalShortcuts(state, editor, io, currentFontIndex);

    ClampFontIndex(currentFontIndex, (int)editorFonts.size());
    RenderMenuBar(state, editor, markdownDef, plainTextDef, menuFont,
                  clear_color, currentFontIndex);

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    RenderEditorWindow(editor, editorFonts, currentFontIndex, windowFlags,
                       state.focusEditorOnStart);
    RenderFindPanel(state, editor, viewport);

    ImGui::Render();
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);

    HandleFileDialogs(state, editor);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}