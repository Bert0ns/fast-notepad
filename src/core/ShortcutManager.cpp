#include "ShortcutManager.h"

constexpr int kDefaultFontIndex = 3;

void ShortcutManager::Handle(AppState& state, TextEditor& editor,
                             FindPanel& findPanel) {
  ImGuiIO& io = ImGui::GetIO();
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false))
    state.triggerOpen = true;
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
    if (io.KeyShift)
      state.triggerSaveAs = true;
    else
      state.triggerSave = true;
  }
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false)) {
    findPanel.Open(editor);
  }
  if (io.KeyCtrl && io.MouseWheel != 0.0f) {
    if (io.MouseWheel > 0)
      state.currentFontIndex++;
    else
      state.currentFontIndex--;
  }
  if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Equal, false) ||
                     ImGui::IsKeyPressed(ImGuiKey_KeypadAdd, false) ||
                     ImGui::IsKeyPressed(ImGuiKey_RightBracket, false)))
    state.currentFontIndex++;
  if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Minus, false) ||
                     ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract, false)))
    state.currentFontIndex--;
  if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_0, false))
    state.currentFontIndex = kDefaultFontIndex;

  if (findPanel.IsOpen() && ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
    findPanel.Close();
  }
}
