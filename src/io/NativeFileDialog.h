#pragma once
#include "IFileDialog.h"

class NativeFileDialog : public IFileDialog {
 public:
  std::string OpenFile() override;
  std::string SaveFile() override;
};
