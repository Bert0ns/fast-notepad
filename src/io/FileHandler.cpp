#include "FileHandler.h"

#include <fstream>
#include <sstream>

FileHandler::FileHandler(IAppFileDialog* dialogs) : m_dialogs(dialogs) {}

std::future<std::optional<std::string>> FileHandler::LoadFileAsync(
    const std::string& filepath) {
  return std::async(
      std::launch::async, [filepath]() -> std::optional<std::string> {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
          std::streamsize size = file.tellg();
          file.seekg(0, std::ios::beg);
          std::string buffer(size, '\0');
          if (file.read(&buffer[0], size)) {
            return buffer;
          }
        }
        return std::nullopt;
      });
}

std::future<bool> FileHandler::SaveFileAsync(const std::string& filepath,
                                             std::string content) {
  return std::async(std::launch::async,
                    [filepath, content = std::move(content)]() {
                      std::ofstream file(filepath);
                      if (file.is_open()) {
                        file << content;
                        return true;
                      }
                      return false;
                    });
}
