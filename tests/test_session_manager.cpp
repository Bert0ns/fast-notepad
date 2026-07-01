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
  std::vector<std::string> testPaths = {"/dummy/path/test1.txt",
                                        "/dummy/path/test2.txt"};

  SessionManager::SaveSessionState(testPaths);

  std::vector<std::string> loadedPaths;
  bool success = SessionManager::LoadSessionState(loadedPaths);

  REQUIRE(success == true);
  REQUIRE(loadedPaths.size() == testPaths.size());
  REQUIRE(loadedPaths[0] == testPaths[0]);
  REQUIRE(loadedPaths[1] == testPaths[1]);
}
