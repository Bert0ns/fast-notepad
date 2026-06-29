#pragma once
#include <string>

class IAppFileDialog {
 public:
  virtual ~IAppFileDialog() = default;
  virtual std::string OpenFile() = 0;
  virtual std::string SaveFile() = 0;
};
