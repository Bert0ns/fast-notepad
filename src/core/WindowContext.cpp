#include "WindowContext.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "Platform.h"

WindowContext::WindowContext() = default;

WindowContext::~WindowContext() {
  if (m_window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }
}

bool WindowContext::Init(int width, int height, const char* title) {
  if (!glfwInit()) return false;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  m_window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!m_window) {
    glfwTerminate();
    return false;
  }

  Platform::SetWindowIcon(m_window);

  glfwMakeContextCurrent(m_window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(m_window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  return true;
}

void WindowContext::PollEvents() {
  glfwPollEvents();
}

void WindowContext::SwapBuffers() {
  glfwSwapBuffers(m_window);
}

bool WindowContext::ShouldClose() const {
  return glfwWindowShouldClose(m_window);
}

void WindowContext::SetShouldClose(bool close) {
  glfwSetWindowShouldClose(m_window, close);
}

void WindowContext::SetWindowTitle(const char* title) {
  glfwSetWindowTitle(m_window, title);
}
