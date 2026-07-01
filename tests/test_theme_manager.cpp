#include <imgui.h>

#include <catch2/catch_test_macros.hpp>

#include "TextEditor.h"
#include "ThemeManager.h"

TEST_CASE("ThemeManager Initialization and Apply", "[ThemeManager]") {
  ImGui::CreateContext();

  ThemeManager themeManager;
  TextEditor editor;

  SECTION("Apply Dark Theme") {
    themeManager.ApplyTheme(true, editor);
    ImVec4 clearColor = themeManager.GetClearColor();
    // Just verify clear color gets set for dark theme
    REQUIRE(clearColor.x == 0.12f);
    REQUIRE(clearColor.y == 0.12f);
    REQUIRE(clearColor.z == 0.12f);
  }

  SECTION("Apply Light Theme") {
    themeManager.ApplyTheme(false, editor);
    ImVec4 clearColor = themeManager.GetClearColor();
    // Verify clear color gets set for light theme
    REQUIRE(clearColor.x == 0.95f);
    REQUIRE(clearColor.y == 0.95f);
    REQUIRE(clearColor.z == 0.95f);
  }

  SECTION("Apply Markdown Mode") {
    // We can't easily extract the exact definition back without getters,
    // but we can ensure it doesn't crash when applying.
    themeManager.ApplyLanguage(AppState::Language::Markdown, editor);
    REQUIRE(editor.GetLanguageDefinition().mName == "Markdown");

    themeManager.ApplyLanguage(AppState::Language::None, editor);
    REQUIRE(editor.GetLanguageDefinition().mName == "Plain Text");
  }

  ImGui::DestroyContext();
}
