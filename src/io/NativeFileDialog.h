#pragma once
#include "IFileDialog.h"

class NativeFileDialog : public IAppFileDialog {
 public:
  std::string OpenFile() override;
  std::string SaveFile() override;
};
