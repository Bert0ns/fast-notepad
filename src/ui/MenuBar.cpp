#include "MenuBar.h"

constexpr float kMenuBarPaddingX = 12.0f;
constexpr float kMenuBarPaddingY = 12.0f;
constexpr int kDefaultFontIndex = 3;

void MenuBar::Render(AppState& state, AppState::Language& currentLanguage,
                     TextEditor& editor, ThemeManager& themeManager,
                     FindPanel& findPanel, WindowContext& windowCtx,
                     ImFont* menuFont) {
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                      ImVec2(kMenuBarPaddingX, kMenuBarPaddingY));
  if (ImGui::BeginMainMenuBar()) {
    if (menuFont) ImGui::PushFont(menuFont);

    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New", "Ctrl+N")) {
        state.triggerNew = true;
      }
      if (ImGui::MenuItem("Open", "Ctrl+O")) state.triggerOpen = true;
      if (ImGui::MenuItem("Save", "Ctrl+S")) state.triggerSave = true;
      if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
        state.triggerSaveAs = true;
      ImGui::Separator();
      if (ImGui::MenuItem("Exit")) state.triggerExit = true;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      if (ImGui::MenuItem("Undo", "Ctrl+Z", nullptr, editor.CanUndo()))
        editor.Undo();
      if (ImGui::MenuItem("Redo", "Ctrl+Y", nullptr, editor.CanRedo()))
        editor.Redo();
      ImGui::Separator();
      if (ImGui::MenuItem("Cut", "Ctrl+X", nullptr, editor.HasSelection()))
        editor.Cut();
      if (ImGui::MenuItem("Copy", "Ctrl+C", nullptr, editor.HasSelection()))
        editor.Copy();
      if (ImGui::MenuItem("Paste", "Ctrl+V")) editor.Paste();
      ImGui::Separator();
      if (ImGui::MenuItem("Find", "Ctrl+F")) findPanel.Open(editor);
      if (ImGui::MenuItem("Replace", "Ctrl+H")) findPanel.Open(editor, true);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      if (ImGui::BeginMenu("Language")) {
        AppState::Language prevLanguage = currentLanguage;
        if (ImGui::MenuItem("None", nullptr,
                            currentLanguage == AppState::Language::None))
          currentLanguage = AppState::Language::None;
        if (ImGui::MenuItem("C++", nullptr,
                            currentLanguage == AppState::Language::Cpp))
          currentLanguage = AppState::Language::Cpp;
        if (ImGui::MenuItem("GLSL", nullptr,
                            currentLanguage == AppState::Language::GLSL))
          currentLanguage = AppState::Language::GLSL;
        if (ImGui::MenuItem("JSON", nullptr,
                            currentLanguage == AppState::Language::Json))
          currentLanguage = AppState::Language::Json;
        if (ImGui::MenuItem("Lua", nullptr,
                            currentLanguage == AppState::Language::Lua))
          currentLanguage = AppState::Language::Lua;
        if (ImGui::MenuItem("Markdown", nullptr,
                            currentLanguage == AppState::Language::Markdown))
          currentLanguage = AppState::Language::Markdown;

        if (currentLanguage != prevLanguage) {
          themeManager.ApplyLanguage(currentLanguage, editor);
        }
        ImGui::EndMenu();
      }

      ImGui::Separator();

      bool prevTheme = state.isDarkTheme;
      ImGui::MenuItem("Dark Theme", nullptr, &state.isDarkTheme);
      if (state.isDarkTheme != prevTheme) {
        themeManager.ApplyTheme(state.isDarkTheme, editor);
      }

      if (ImGui::MenuItem("Zoom In", "Ctrl++")) state.currentFontIndex++;
      if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) state.currentFontIndex--;
      if (ImGui::MenuItem("Reset Zoom", "Ctrl+0"))
        state.currentFontIndex = kDefaultFontIndex;

      ImGui::EndMenu();
    }
    if (menuFont) ImGui::PopFont();
    ImGui::EndMainMenuBar();
  }
  ImGui::PopStyleVar();
}
