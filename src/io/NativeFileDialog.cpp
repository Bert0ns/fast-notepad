#include "NativeFileDialog.h"
#include "portable-file-dialogs.h"

std::string NativeFileDialog::OpenFile() {
  auto f = pfd::open_file(
      "Choose text file", ".",
      {"Text Files", "*.txt *.md *.cpp *.h", "All Files", "*"});
  if (!f.result().empty()) {
    return f.result()[0];
  }
  return "";
}

std::string NativeFileDialog::SaveFile() {
  auto f = pfd::save_file("Save file", ".", {"Text Files", "*.txt *.md"});
  return f.result();
}
