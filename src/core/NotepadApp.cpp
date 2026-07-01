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

  std::vector<std::string> filePaths;
  if (SessionManager::LoadSessionState(filePaths) && !filePaths.empty()) {
    for (const auto& fp : filePaths) {
      AddNewTab(fp);
      auto tab = m_tabs.back().get();
      tab->isLoading = true;
      tab->loadFuture = m_fileHandler.LoadFileAsync(fp);
    }
    m_activeTabIndex = 0;
  } else {
    AddNewTab();
    m_tabs[0]->editor.SetText("Start *typing* or open a file...");
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
  auto tab = GetActiveTab();
  if (!tab) return;
  const auto lines = tab->editor.GetTextLines();
  if (lines.empty()) {
    tab->editor.SetCursorPosition({0, 0});
    return;
  }
  const int lastLine = static_cast<int>(lines.size() - 1);

  int endCol = 0;
  const std::string& line = lines[lastLine];
  int currentColumn = 0;
  int i = 0;
  int tabSize = tab->editor.GetTabSize();
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

  tab->editor.SetSelection({0, 0}, {lastLine, endCol});
  tab->editor.SetCursorPosition({lastLine, endCol});
}

EditorTab* NotepadApp::GetActiveTab() {
  if (m_activeTabIndex >= 0 && m_activeTabIndex < (int)m_tabs.size()) {
    return m_tabs[m_activeTabIndex].get();
  }
  return nullptr;
}

void NotepadApp::AddNewTab(const std::string& filePath) {
  auto tab = std::make_unique<EditorTab>();
  tab->currentFilePath = filePath;
  m_themeManager.ApplyTheme(m_state.isDarkTheme, tab->editor);
  m_themeManager.ApplyMarkdownMode(m_state.enableMarkdown, tab->editor);
  m_tabs.push_back(std::move(tab));
  m_state.forceSelectTab = (int)m_tabs.size() - 1;
  m_activeTabIndex = (int)m_tabs.size() - 1;
}

void NotepadApp::CloseTab(int index) {
  if (index >= 0 && index < (int)m_tabs.size()) {
    m_tabs.erase(m_tabs.begin() + index);
    if (m_activeTabIndex >= (int)m_tabs.size()) {
      m_activeTabIndex = m_tabs.size() - 1;
    }
    if (m_tabs.empty()) {
      AddNewTab();
    }
  }
}

void NotepadApp::ExecuteCloseTab(int index) { CloseTab(index); }

void NotepadApp::ExecuteNew() { AddNewTab(); }

void NotepadApp::ExecuteOpen() {
  std::string result = m_nativeDialogs.OpenFile();
  if (!result.empty()) {
    AddNewTab(result);
    auto tab = GetActiveTab();
    tab->isLoading = true;
    tab->loadFuture = m_fileHandler.LoadFileAsync(result);
  }
}

void NotepadApp::ExecuteExit() { m_windowCtx.SetShouldClose(true); }

void NotepadApp::HandlePendingAction() {
  switch (m_state.pendingAction) {
    case AppState::PendingAction::CloseTab:
      ExecuteCloseTab(m_pendingCloseTabIndex);
      break;
    case AppState::PendingAction::Exit:
      ExecuteExit();
      break;
    default:
      break;
  }
  m_state.pendingAction = AppState::PendingAction::None;
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

  if (ImGui::BeginTabBar(
          "EditorTabs",
          ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {
    for (int i = 0; i < (int)m_tabs.size(); ++i) {
      auto& tab = m_tabs[i];
      std::string title =
          tab->currentFilePath.empty() ? "Untitled" : tab->currentFilePath;
      size_t slash = title.find_last_of("/\\");
      if (slash != std::string::npos) title = title.substr(slash + 1);
      if (tab->isDirty) title += "*";
      title += "###tab" + std::to_string(i);  // Unique ID

      bool open = true;
      ImGuiTabItemFlags flags =
          (m_state.forceSelectTab == i) ? ImGuiTabItemFlags_SetSelected : 0;
      if (ImGui::BeginTabItem(title.c_str(), &open, flags)) {
        if (m_state.forceSelectTab == i) {
          m_state.forceSelectTab = -1;  // reset after forcing
        }
        m_activeTabIndex = i;
        tab->editor.Render("TextEditor");
        ImGui::EndTabItem();
      }

      if (!open) {
        // Tab closed button clicked
        if (tab->isDirty) {
          m_state.pendingAction = AppState::PendingAction::CloseTab;
          m_pendingCloseTabIndex = i;
          m_state.showUnsavedChangesModal = true;
        } else {
          ExecuteCloseTab(i);
          --i;  // Adjust index after erase
        }
      }
    }
    ImGui::EndTabBar();
  }

  if (m_state.currentFontIndex >= 0 &&
      m_state.currentFontIndex < (int)m_editorFonts.size() &&
      m_editorFonts[m_state.currentFontIndex]) {
    ImGui::PopFont();
  }
  ImGui::End();
  ImGui::PopStyleVar();
}

void NotepadApp::UpdateWindowTitle() {
  if (GetActiveTab() &&
      GetActiveTab()->currentFilePath != GetActiveTab()->lastFilePath) {
    std::string title =
        GetActiveTab()->currentFilePath.empty()
            ? "Fast Notepad - Untitled"
            : "Fast Notepad - " + GetActiveTab()->currentFilePath;
    m_windowCtx.SetWindowTitle(title.c_str());
    GetActiveTab()->lastFilePath = GetActiveTab()->currentFilePath;
  }
}

int NotepadApp::Run() {
  while (!m_windowCtx.ShouldClose() || m_state.triggerExit) {
    if (m_windowCtx.ShouldClose()) {
      m_windowCtx.SetShouldClose(false);
      m_state.triggerExit = true;
    }

    m_windowCtx.PollEvents();
    UpdateWindowTitle();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (GetActiveTab())
      m_shortcutManager.Handle(m_state, GetActiveTab()->editor, m_findPanel);
    if (m_state.currentFontIndex < 0) m_state.currentFontIndex = 0;
    if (m_state.currentFontIndex >= (int)m_editorFonts.size())
      m_state.currentFontIndex = (int)m_editorFonts.size() - 1;

    if (GetActiveTab())
      m_menuBar.Render(m_state, GetActiveTab()->editor, m_themeManager,
                       m_findPanel, m_windowCtx, m_menuFont);

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    RenderEditor();
    if (GetActiveTab()) m_findPanel.Render(GetActiveTab()->editor, viewport);

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

    for (auto& tab : m_tabs) {
      if (tab->isLoading && tab->loadFuture.valid()) {
        if (tab->loadFuture.wait_for(std::chrono::seconds(0)) ==
            std::future_status::ready) {
          auto result = tab->loadFuture.get();
          if (result.has_value()) {
            tab->editor.SetText(result.value());
            tab->currentFilePath = tab->pendingFilePath;
            tab->isDirty = false;
          } else {
            m_state.showErrorPopup = true;
            m_state.errorMessage =
                "Failed to load file: " + tab->pendingFilePath;
          }
          tab->isLoading = false;
        }
      }

      if (tab->isSaving && tab->saveFuture.valid()) {
        if (tab->saveFuture.wait_for(std::chrono::seconds(0)) ==
            std::future_status::ready) {
          bool result = tab->saveFuture.get();
          if (result) {
            tab->currentFilePath = tab->pendingFilePath;
            tab->isDirty = false;
            HandlePendingAction();
          } else {
            m_state.showErrorPopup = true;
            m_state.errorMessage =
                "Failed to save file: " + tab->pendingFilePath;
          }
          tab->isSaving = false;
        }
      }
    }

    if (GetActiveTab() && !GetActiveTab()->isLoading &&
        !GetActiveTab()->isSaving) {
      if (m_state.triggerNew) {
        ExecuteNew();
        m_state.triggerNew = false;
      }
      if (m_state.triggerOpen) {
        ExecuteOpen();
        m_state.triggerOpen = false;
      }
      if (m_state.triggerExit) {
        ExecuteExit();
        m_state.triggerExit = false;
      }
      if (m_state.triggerSave) {
        if (GetActiveTab()->currentFilePath.empty()) {
          m_state.triggerSaveAs = true;
        } else {
          GetActiveTab()->pendingFilePath = GetActiveTab()->currentFilePath;
          GetActiveTab()->isSaving = true;
          GetActiveTab()->saveFuture =
              m_fileHandler.SaveFileAsync(GetActiveTab()->pendingFilePath,
                                          GetActiveTab()->editor.GetText());
        }
        m_state.triggerSave = false;
      }
      if (m_state.triggerSaveAs) {
        std::string result = m_nativeDialogs.SaveFile();
        if (!result.empty()) {
          GetActiveTab()->pendingFilePath = result;
          GetActiveTab()->isSaving = true;
          GetActiveTab()->saveFuture = m_fileHandler.SaveFileAsync(
              result, GetActiveTab()->editor.GetText());
        }
        m_state.triggerSaveAs = false;
      }
    }

    if (GetActiveTab() &&
        (GetActiveTab()->isLoading || GetActiveTab()->isSaving)) {
      ImGui::OpenPopup("Progress");
      if (ImGui::BeginPopupModal("Progress", NULL,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoTitleBar)) {
        ImGui::Text(GetActiveTab()->isLoading ? "Loading..." : "Saving...");
        ImGui::EndPopup();
      }
    }

    if (m_state.showUnsavedChangesModal) {
      ImGui::OpenPopup("Unsaved Changes");
      m_state.showUnsavedChangesModal = false;
    }
    if (ImGui::BeginPopupModal("Unsaved Changes", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("You have unsaved changes. Would you like to save them?");
      ImGui::Separator();

      if (ImGui::Button("Save", ImVec2(120, 0))) {
        m_state.triggerSave = true;
        ImGui::CloseCurrentPopup();
      }
      ImGui::SetItemDefaultFocus();  // "Save" or "Don't Save"? Plan says "Don't
                                     // Save" comes as pre-selected
      ImGui::SameLine();

      // Wait, Plan says: "Don't Save: Discard changes and proceed with the
      // intercepted action. comes as pre-selected"
      if (ImGui::Button("Don't Save", ImVec2(120, 0))) {
        GetActiveTab()->isDirty = false;
        HandlePendingAction();
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::IsItemFocused() == false && ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere(-1);  // Make "Don't Save" pre-selected
      }

      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        m_state.pendingAction = AppState::PendingAction::None;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    if (GetActiveTab() && GetActiveTab()->editor.IsTextChanged()) {
      GetActiveTab()->isDirty = true;
    }

    ImGui::Render();
    ImVec4 clearColor = m_themeManager.GetClearColor();
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    m_windowCtx.SwapBuffers();
  }

  AppSettings settings;
  settings.enableMarkdown = m_state.enableMarkdown;
  settings.isDarkTheme = m_state.isDarkTheme;
  settings.currentFontIndex = m_state.currentFontIndex;
  SessionManager::SaveSettings(settings);

  std::vector<std::string> filePaths;
  for (auto& tab : m_tabs) {
    if (!tab->currentFilePath.empty()) {
      filePaths.push_back(tab->currentFilePath);
    }
  }
  SessionManager::SaveSessionState(filePaths);
  return 0;
}
