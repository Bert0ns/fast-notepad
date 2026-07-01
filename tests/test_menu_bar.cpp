#include <imgui.h>

#include <catch2/catch_test_macros.hpp>

#include "AppState.h"
#include "FindPanel.h"
#include "MenuBar.h"
#include "TextEditor.h"
#include "ThemeManager.h"
#include "WindowContext.h"

TEST_CASE("MenuBar Render", "[MenuBar]") {
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2(800, 600);

  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  MenuBar menuBar;
  AppState state;
  TextEditor editor;
  ThemeManager themeManager;
  FindPanel findPanel;
  WindowContext windowCtx;

  // Wait, test_find_panel.cpp didn't need WindowContext.
  // MenuBar takes a WindowContext& just to call SetShouldClose(true) on Exit.

  // We can just pass a dummy WindowContext but WindowContext constructor calls
  // glfwInit(). glfwInit() usually works in tests if we have X11 or Wayland,
  // which we do since test_platform passed!

  ImGui::NewFrame();

  // We just render it to make sure it doesn't crash.
  // But testing the internals of MenuBar rendering is hard without clicking.
  AppState::Language currentLanguage = AppState::Language::None;
  REQUIRE_NOTHROW(menuBar.Render(state, currentLanguage, editor, themeManager,
                                 findPanel, windowCtx, nullptr));

  ImGui::EndFrame();
  ImGui::DestroyContext();
}
