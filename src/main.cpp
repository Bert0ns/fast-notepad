#include "NotepadApp.h"

int main() {
  NotepadApp app;
  if (!app.Init()) {
    return -1;
  }
  return app.Run();
}
