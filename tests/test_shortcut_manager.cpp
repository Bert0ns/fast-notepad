#include <imgui.h>

#include <catch2/catch_test_macros.hpp>

#include "AppState.h"
#include "FindPanel.h"
#include "ShortcutManager.h"
#include "TextEditor.h"

TEST_CASE("ShortcutManager Logic", "[ShortcutManager]") {
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2(800, 600);

  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  ShortcutManager manager;
  AppState state;
  TextEditor editor;
  FindPanel findPanel;

  // Clear initial state
  state.triggerOpen = false;
  state.triggerSave = false;
  state.triggerSaveAs = false;

  SECTION("Ctrl+O sets triggerOpen") {
    io.AddKeyEvent(ImGuiKey_ModCtrl, true);
    io.AddKeyEvent(ImGuiKey_O, true);
    ImGui::NewFrame();
    manager.Handle(state, editor, findPanel);
    REQUIRE(state.triggerOpen == true);
    ImGui::EndFrame();
    io.AddKeyEvent(ImGuiKey_ModCtrl, false);
    io.AddKeyEvent(ImGuiKey_O, false);
  }

  SECTION("Ctrl+S sets triggerSave") {
    io.AddKeyEvent(ImGuiKey_ModCtrl, true);
    io.AddKeyEvent(ImGuiKey_ModShift, false);
    io.AddKeyEvent(ImGuiKey_S, true);
    ImGui::NewFrame();
    manager.Handle(state, editor, findPanel);
    REQUIRE(state.triggerSave == true);
    REQUIRE(state.triggerSaveAs == false);
    ImGui::EndFrame();
    io.AddKeyEvent(ImGuiKey_ModCtrl, false);
    io.AddKeyEvent(ImGuiKey_S, false);
  }

  SECTION("Ctrl+Shift+S sets triggerSaveAs") {
    io.AddKeyEvent(ImGuiKey_ModCtrl, true);
    io.AddKeyEvent(ImGuiKey_ModShift, true);
    io.AddKeyEvent(ImGuiKey_S, true);
    ImGui::NewFrame();
    manager.Handle(state, editor, findPanel);
    REQUIRE(state.triggerSaveAs == true);
    REQUIRE(state.triggerSave == false);
    ImGui::EndFrame();
    io.AddKeyEvent(ImGuiKey_ModCtrl, false);
    io.AddKeyEvent(ImGuiKey_ModShift, false);
    io.AddKeyEvent(ImGuiKey_S, false);
  }

  SECTION("Ctrl+F opens find panel") {
    io.AddKeyEvent(ImGuiKey_ModCtrl, true);
    io.AddKeyEvent(ImGuiKey_F, true);
    REQUIRE(findPanel.IsOpen() == false);
    ImGui::NewFrame();
    manager.Handle(state, editor, findPanel);
    REQUIRE(findPanel.IsOpen() == true);
    ImGui::EndFrame();
    io.AddKeyEvent(ImGuiKey_ModCtrl, false);
    io.AddKeyEvent(ImGuiKey_F, false);
  }

  SECTION("Ctrl+0 resets font size") {
    state.currentFontIndex = 5;
    io.AddKeyEvent(ImGuiKey_ModCtrl, true);
    io.AddKeyEvent(ImGuiKey_0, true);
    ImGui::NewFrame();
    manager.Handle(state, editor, findPanel);
    REQUIRE(state.currentFontIndex == 3);  // kDefaultFontIndex
    ImGui::EndFrame();
    io.AddKeyEvent(ImGuiKey_ModCtrl, false);
    io.AddKeyEvent(ImGuiKey_0, false);
  }

  SECTION("Ctrl+Plus increases font size") {
    state.currentFontIndex = 3;
    io.AddKeyEvent(ImGuiKey_ModCtrl, true);
    io.AddKeyEvent(ImGuiKey_Equal, true);
    ImGui::NewFrame();
    manager.Handle(state, editor, findPanel);
    REQUIRE(state.currentFontIndex == 4);
    ImGui::EndFrame();
    io.AddKeyEvent(ImGuiKey_ModCtrl, false);
    io.AddKeyEvent(ImGuiKey_Equal, false);
  }

  ImGui::DestroyContext();
}
