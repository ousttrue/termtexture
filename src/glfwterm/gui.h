#pragma once
#include <functional>
#include <list>
#include <string_view>

class Gui {
  struct GuiWindow {
    std::function<void(bool *pShow)> on_updated;
    bool show = true;
    bool use_show = true;
    void Update() {
      if (!use_show) {
        on_updated(nullptr);
      } else if (show) {
        on_updated(&show);
      }
    }
  };
  std::list<GuiWindow> windows_;

public:
  float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

  Gui(struct GLFWwindow *window, std::string_view glsl_version);
  ~Gui();
  void UpdateRender();
};
