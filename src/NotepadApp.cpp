#include "NotepadApp.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Platform.h"
#include "SessionManager.h"

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

  m_window =
      glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, NULL, NULL);
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
    if (c == '\t') {
      currentColumn = (currentColumn / tabSize) * tabSize + tabSize;
      ++i;
    } else {
      ++currentColumn;
      int len = 1;
      if ((c & 0xFE) == 0xFC)
        len = 6;
      else if ((c & 0xFC) == 0xF8)
        len = 5;
      else if ((c & 0xF8) == 0xF0)
        len = 4;
      else if ((c & 0xF0) == 0xE0)
        len = 3;
      else if ((c & 0xE0) == 0xC0)
        len = 2;
      i += len;
    }
  }
  endCol = currentColumn;

  m_editor.SetSelection({0, 0}, {lastLine, endCol});
  m_editor.SetCursorPosition({lastLine, endCol});
}

void NotepadApp::HandleShortcuts() {
  ImGuiIO& io = ImGui::GetIO();
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false))
    m_triggerOpen = true;
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
    if (io.KeyShift)
      m_triggerSaveAs = true;
    else
      m_triggerSave = true;
  }
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false)) {
    m_findPanel.Open(m_editor);
  }
  if (io.KeyCtrl && io.MouseWheel != 0.0f) {
    if (io.MouseWheel > 0)
      m_currentFontIndex++;
    else
      m_currentFontIndex--;
  }
  if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Equal, false) ||
                     ImGui::IsKeyPressed(ImGuiKey_KeypadAdd, false) ||
                     ImGui::IsKeyPressed(ImGuiKey_RightBracket, false)))
    m_currentFontIndex++;
  if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Minus, false) ||
                     ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract, false)))
    m_currentFontIndex--;
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_0, false))
    m_currentFontIndex = kDefaultFontIndex;

  if (m_findPanel.IsOpen() && ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
    m_findPanel.Close();
  }
}

void NotepadApp::RenderMenuBar() {
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                      ImVec2(kMenuBarPaddingX, kMenuBarPaddingY));
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
      if (ImGui::MenuItem("Undo", "Ctrl+Z", nullptr, m_editor.CanUndo()))
        m_editor.Undo();
      if (ImGui::MenuItem("Redo", "Ctrl+Y", nullptr, m_editor.CanRedo()))
        m_editor.Redo();
      ImGui::Separator();
      if (ImGui::MenuItem("Cut", "Ctrl+X", nullptr, m_editor.HasSelection()))
        m_editor.Cut();
      if (ImGui::MenuItem("Copy", "Ctrl+C", nullptr, m_editor.HasSelection()))
        m_editor.Copy();
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
      if (ImGui::MenuItem("Reset Zoom", "Ctrl+0"))
        m_currentFontIndex = kDefaultFontIndex;

      ImGui::EndMenu();
    }
    if (m_menuFont) ImGui::PopFont();
    ImGui::EndMainMenuBar();
  }
  ImGui::PopStyleVar();
}

void NotepadApp::RenderEditor() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGuiWindowFlags windowFlags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoBringToFrontOnFocus;
  ImGui::Begin("MainWorkspace", nullptr, windowFlags);

  if (m_focusEditorOnStart) {
    ImGui::SetKeyboardFocusHere();
    m_focusEditorOnStart = false;
  }

  if (m_currentFontIndex >= 0 &&
      m_currentFontIndex < (int)m_editorFonts.size() &&
      m_editorFonts[m_currentFontIndex]) {
    ImGui::PushFont(m_editorFonts[m_currentFontIndex]);
  }

  m_editor.Render("TextEditor");

  if (m_currentFontIndex >= 0 &&
      m_currentFontIndex < (int)m_editorFonts.size() &&
      m_editorFonts[m_currentFontIndex]) {
    ImGui::PopFont();
  }
  ImGui::End();
  ImGui::PopStyleVar();
}

void NotepadApp::UpdateWindowTitle() {
  if (m_currentFilePath != m_lastFilePath) {
    std::string title = m_currentFilePath.empty()
                            ? "Fast Notepad - Untitled"
                            : "Fast Notepad - " + m_currentFilePath;
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
    if (m_currentFontIndex >= (int)m_editorFonts.size())
      m_currentFontIndex = (int)m_editorFonts.size() - 1;

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

    m_fileHandler.HandleDialogs(m_currentFilePath, m_editor, m_triggerOpen,
                                m_triggerSave, m_triggerSaveAs);
  }

  SessionManager::SaveSessionState(m_currentFilePath, m_editor.GetText());
  return 0;
}
