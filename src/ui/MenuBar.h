#pragma once
#include <imgui.h>

#include "AppState.h"
#include "FindPanel.h"
#include "TextEditor.h"
#include "ThemeManager.h"
#include "WindowContext.h"

class MenuBar {
 public:
  void Render(AppState& state, TextEditor& editor, ThemeManager& themeManager,
              FindPanel& findPanel, WindowContext& windowCtx, ImFont* menuFont);
};
