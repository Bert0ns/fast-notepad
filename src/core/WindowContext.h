#pragma once

struct GLFWwindow;

class WindowContext {
 public:
  WindowContext();
  ~WindowContext();

  bool Init(int width, int height, const char* title);
  void PollEvents();
  void SwapBuffers();
  bool ShouldClose() const;
  void SetShouldClose(bool close);
  void SetWindowTitle(const char* title);
  
  GLFWwindow* GetWindow() const { return m_window; }

 private:
  GLFWwindow* m_window = nullptr;
};
