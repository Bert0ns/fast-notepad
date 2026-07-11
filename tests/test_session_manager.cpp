#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include "SessionManager.h"

TEST_CASE("SessionManager Settings Save and Load", "[SessionManager]") {
  AppSettings originalSettings;
  originalSettings.isDarkTheme = false;
  originalSettings.currentFontIndex = 5;

  SessionManager::SaveSettings(originalSettings);

  AppSettings loadedSettings;
  loadedSettings.isDarkTheme = true;
  loadedSettings.currentFontIndex = 0;

  bool success = SessionManager::LoadSettings(loadedSettings);

  REQUIRE(success == true);
  REQUIRE(loadedSettings.isDarkTheme == originalSettings.isDarkTheme);
  REQUIRE(loadedSettings.currentFontIndex == originalSettings.currentFontIndex);
}

TEST_CASE("SessionManager State Save and Load", "[SessionManager]") {
  std::vector<SessionTab> testTabs = {
      {"/dummy/path/test1.txt", false, ""},
      {"", true, "unsaved content test"},
      {"/dummy/path/test2.txt", true, "dirty file content"}};

  SessionManager::SaveSessionState(testTabs);

  std::vector<SessionTab> loadedTabs;
  bool success = SessionManager::LoadSessionState(loadedTabs);

  REQUIRE(success == true);
  REQUIRE(loadedTabs.size() == testTabs.size());

  REQUIRE(loadedTabs[0].filePath == testTabs[0].filePath);
  REQUIRE(loadedTabs[0].isDirty == testTabs[0].isDirty);

  REQUIRE(loadedTabs[1].filePath == testTabs[1].filePath);
  REQUIRE(loadedTabs[1].isDirty == testTabs[1].isDirty);
  REQUIRE(loadedTabs[1].content == testTabs[1].content);

  REQUIRE(loadedTabs[2].filePath == testTabs[2].filePath);
  REQUIRE(loadedTabs[2].isDirty == testTabs[2].isDirty);
  REQUIRE(loadedTabs[2].content == testTabs[2].content);
}
