#pragma once
#include <imgui.h>

#include "AppState.h"
#include "FindPanel.h"
#include "TextEditor.h"

class ShortcutManager {
 public:
  void Handle(AppState& state, TextEditor& editor, FindPanel& findPanel);
};
