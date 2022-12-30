#pragma once
#include <string_view>

class Gui {
  // Our state
  bool show_demo_window_ = true;
  bool show_another_window_ = false;

public:
  float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

  Gui(struct GLFWwindow *window, std::string_view glsl_version);
  ~Gui();
  void UpdateRender();
};
