#pragma once
#include <chrono>
#include <functional>
#include <list>
#include <ratio>
#include <string_view>

class Gui {
  struct GuiWindow {

    std::function<void(bool *pShow)> on_show;
    bool show = true;
    bool use_show = true;
    void Show() {
      if (!use_show) {
        on_show(nullptr);
      } else if (show) {
        on_show(&show);
      }
    }

    std::function<void(std::chrono::nanoseconds time)>
        on_update;
    void Update(std::chrono::nanoseconds time) {
      if (on_update) {
        on_update(time);
      }
    }
  };
  std::list<GuiWindow> windows_;

public:
  float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};
  Gui();
  ~Gui();
  Gui(const Gui &) = delete;
  Gui &operator=(const Gui &) = delete;
  bool Initialize(struct GLFWwindow *window, std::string_view glsl_version,
      const std::string &fontfile);
  void UpdateRender(std::chrono::nanoseconds time);
};
