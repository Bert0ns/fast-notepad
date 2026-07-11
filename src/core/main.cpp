#include "NotepadApp.h"

int main(int argc, char** argv) {
  NotepadApp app;
  if (!app.Init(argc, argv)) {
    return -1;
  }
  return app.Run();
}
