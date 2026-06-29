#pragma once
#include <string>

class IFileDialog {
 public:
  virtual ~IFileDialog() = default;
  virtual std::string OpenFile() = 0;
  virtual std::string SaveFile() = 0;
};
