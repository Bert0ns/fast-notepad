#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include "SessionManager.h"

TEST_CASE("SessionManager Settings Save and Load", "[SessionManager]") {
  AppSettings originalSettings;
  originalSettings.enableMarkdown = false;
  originalSettings.isDarkTheme = false;
  originalSettings.currentFontIndex = 5;

  SessionManager::SaveSettings(originalSettings);

  AppSettings loadedSettings;
  // Set some defaults to make sure they get overwritten
  loadedSettings.enableMarkdown = true;
  loadedSettings.isDarkTheme = true;
  loadedSettings.currentFontIndex = 0;

  bool success = SessionManager::LoadSettings(loadedSettings);

  REQUIRE(success == true);
  REQUIRE(loadedSettings.enableMarkdown == originalSettings.enableMarkdown);
  REQUIRE(loadedSettings.isDarkTheme == originalSettings.isDarkTheme);
  REQUIRE(loadedSettings.currentFontIndex == originalSettings.currentFontIndex);
}

TEST_CASE("SessionManager State Save and Load", "[SessionManager]") {
  std::string testPath = "/dummy/path/test.txt";
  std::string testContent = "Hello, world!\nThis is a test.";

  SessionManager::SaveSessionState(testPath, testContent);

  std::string loadedPath;
  std::string loadedContent;
  bool success = SessionManager::LoadSessionState(loadedPath, loadedContent);

  REQUIRE(success == true);
  REQUIRE(loadedPath == testPath);
  REQUIRE(loadedContent == testContent);
}
