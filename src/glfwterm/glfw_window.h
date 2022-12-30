#pragma once
#include <string>
#include <string_view>

class Window {
  struct GLFWwindow *window_ = nullptr;
  std::string glsl_version_;

public:
  Window();
  ~Window();
  std::string_view glsl_version() const { return glsl_version_; }
  struct GLFWwindow *CreaeWindow(int width, int height, const char *title);
  bool BeginFrame(const float clear_color[4]);
  void EndFrame();
};
