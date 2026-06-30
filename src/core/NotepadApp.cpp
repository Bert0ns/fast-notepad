#define GL_SILENCE_DEPRECATION
#include "NotepadApp.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <chrono>

#include "Platform.h"
#include "SessionManager.h"
#include "Utf8Utils.h"

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

NotepadApp::NotepadApp() : m_fileHandler(&m_nativeDialogs) {}

NotepadApp::~NotepadApp() {}

bool NotepadApp::Init() {
  if (!m_windowCtx.Init(kWindowWidth, kWindowHeight, kWindowTitle)) {
    return false;
  }

  LoadFonts();

  AppSettings settings;
  if (SessionManager::LoadSettings(settings)) {
    m_state.enableMarkdown = settings.enableMarkdown;
    m_state.isDarkTheme = settings.isDarkTheme;
    m_state.currentFontIndex = settings.currentFontIndex;
  }

  m_themeManager.ApplyTheme(m_state.isDarkTheme, m_editor);
  m_themeManager.ApplyMarkdownMode(m_state.enableMarkdown, m_editor);

  std::string content;
  if (SessionManager::LoadSessionState(m_state.currentFilePath, content)) {
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
      i += Utf8Utils::CharLength(c);
    }
  }
  endCol = currentColumn;

  m_editor.SetSelection({0, 0}, {lastLine, endCol});
  m_editor.SetCursorPosition({lastLine, endCol});
}

void NotepadApp::RenderEditor() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGuiWindowFlags windowFlags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoBringToFrontOnFocus;
  ImGui::Begin("MainWorkspace", nullptr, windowFlags);

  if (m_state.focusEditorOnStart) {
    ImGui::SetKeyboardFocusHere();
    m_state.focusEditorOnStart = false;
  }

  if (m_state.currentFontIndex >= 0 &&
      m_state.currentFontIndex < (int)m_editorFonts.size() &&
      m_editorFonts[m_state.currentFontIndex]) {
    ImGui::PushFont(m_editorFonts[m_state.currentFontIndex]);
  }

  m_editor.Render("TextEditor");

  if (m_state.currentFontIndex >= 0 &&
      m_state.currentFontIndex < (int)m_editorFonts.size() &&
      m_editorFonts[m_state.currentFontIndex]) {
    ImGui::PopFont();
  }
  ImGui::End();
  ImGui::PopStyleVar();
}

void NotepadApp::UpdateWindowTitle() {
  if (m_state.currentFilePath != m_state.lastFilePath) {
    std::string title = m_state.currentFilePath.empty()
                            ? "Fast Notepad - Untitled"
                            : "Fast Notepad - " + m_state.currentFilePath;
    m_windowCtx.SetWindowTitle(title.c_str());
    m_state.lastFilePath = m_state.currentFilePath;
  }
}

int NotepadApp::Run() {
  while (!m_windowCtx.ShouldClose()) {
    m_windowCtx.PollEvents();
    UpdateWindowTitle();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_shortcutManager.Handle(m_state, m_editor, m_findPanel);
    if (m_state.currentFontIndex < 0) m_state.currentFontIndex = 0;
    if (m_state.currentFontIndex >= (int)m_editorFonts.size())
      m_state.currentFontIndex = (int)m_editorFonts.size() - 1;

    m_menuBar.Render(m_state, m_editor, m_themeManager, m_findPanel,
                     m_windowCtx, m_menuFont);

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    RenderEditor();
    m_findPanel.Render(m_editor, viewport);

    if (m_state.showErrorPopup) {
      ImGui::OpenPopup("Error");
      m_state.showErrorPopup = false;
    }
    if (ImGui::BeginPopupModal("Error", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("%s", m_state.errorMessage.c_str());
      if (ImGui::Button("OK", ImVec2(120, 0))) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    ImGui::Render();
    ImVec4 clearColor = m_themeManager.GetClearColor();
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    m_windowCtx.SwapBuffers();

    if (m_isLoading && m_loadFuture.valid()) {
      if (m_loadFuture.wait_for(std::chrono::seconds(0)) ==
          std::future_status::ready) {
        auto result = m_loadFuture.get();
        if (result.has_value()) {
          m_editor.SetText(result.value());
          m_state.currentFilePath = m_pendingFilePath;
        } else {
          m_state.showErrorPopup = true;
          m_state.errorMessage = "Failed to load file: " + m_pendingFilePath;
        }
        m_isLoading = false;
      }
    }

    if (m_isSaving && m_saveFuture.valid()) {
      if (m_saveFuture.wait_for(std::chrono::seconds(0)) ==
          std::future_status::ready) {
        bool result = m_saveFuture.get();
        if (result) {
          m_state.currentFilePath = m_pendingFilePath;
        } else {
          m_state.showErrorPopup = true;
          m_state.errorMessage = "Failed to save file: " + m_pendingFilePath;
        }
        m_isSaving = false;
      }
    }

    if (!m_isLoading && !m_isSaving) {
      if (m_state.triggerOpen) {
        std::string result = m_nativeDialogs.OpenFile();
        if (!result.empty()) {
          m_pendingFilePath = result;
          m_isLoading = true;
          m_loadFuture = m_fileHandler.LoadFileAsync(result);
        }
        m_state.triggerOpen = false;
      }
      if (m_state.triggerSave) {
        if (m_state.currentFilePath.empty()) {
          m_state.triggerSaveAs = true;
        } else {
          m_pendingFilePath = m_state.currentFilePath;
          m_isSaving = true;
          m_saveFuture = m_fileHandler.SaveFileAsync(m_pendingFilePath,
                                                     m_editor.GetText());
        }
        m_state.triggerSave = false;
      }
      if (m_state.triggerSaveAs) {
        std::string result = m_nativeDialogs.SaveFile();
        if (!result.empty()) {
          m_pendingFilePath = result;
          m_isSaving = true;
          m_saveFuture =
              m_fileHandler.SaveFileAsync(result, m_editor.GetText());
        }
        m_state.triggerSaveAs = false;
      }
    }

    if (m_isLoading || m_isSaving) {
      ImGui::OpenPopup("Progress");
      if (ImGui::BeginPopupModal("Progress", NULL,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoTitleBar)) {
        ImGui::Text(m_isLoading ? "Loading..." : "Saving...");
        ImGui::EndPopup();
      }
    }
  }

  AppSettings settings;
  settings.enableMarkdown = m_state.enableMarkdown;
  settings.isDarkTheme = m_state.isDarkTheme;
  settings.currentFontIndex = m_state.currentFontIndex;
  SessionManager::SaveSettings(settings);

  SessionManager::SaveSessionState(m_state.currentFilePath, m_editor.GetText());
  return 0;
}
