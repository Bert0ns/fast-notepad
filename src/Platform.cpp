#include "Platform.h"

#include <GLFW/glfw3.h>

#include <filesystem>
#include <fstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>

#include "resource.h"
#endif

std::vector<std::string> Platform::GetIconSearchPaths() {
  std::vector<std::string> paths = {"icon.png", "../icon.png",
                                    "../assets/icon.png"};
  std::error_code ec;
  auto cwd = std::filesystem::current_path(ec);
  if (!ec) {
    paths.push_back((cwd / "icon.png").string());
    paths.push_back((cwd.parent_path() / "icon.png").string());
  }
  return paths;
}

bool Platform::SetWindowIcon(GLFWwindow* window) {
#ifdef _WIN32
  HWND hwnd = glfwGetWin32Window(window);
  if (!hwnd) return false;
  HICON icon = static_cast<HICON>(LoadImageW(GetModuleHandleW(nullptr),
                                             MAKEINTRESOURCEW(IDI_ICON1),
                                             IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
  if (!icon) return false;
  SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
  SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
  return true;
#else
  int width = 0, height = 0, channels = 0;
  for (const auto& path : GetIconSearchPaths()) {
    stbi_uc* pixels =
        stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) continue;
    GLFWimage image;
    image.width = width;
    image.height = height;
    image.pixels = pixels;
    glfwSetWindowIcon(window, 1, &image);
    stbi_image_free(pixels);
    return true;
  }
  return false;
#endif
}

const char* Platform::FindMonospaceFontPath() {
  std::ifstream f_win("C:\\Windows\\Fonts\\consola.ttf");
  if (f_win.good()) return "C:\\Windows\\Fonts\\consola.ttf";
  std::ifstream f_lin("/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf");
  if (f_lin.good()) return "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf";
  return nullptr;
}
