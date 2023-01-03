#pragma once
#include <chrono>
#include <functional>
#include <glo/fbo.h>
#include <imgui.h>
#include <memory>
#include <string>

void another_window(bool *p_open);

struct simple_window {
  std::string name_;
  bool *show_demo_window_;
  bool *show_another_window_;
  float *clear_color;
  void operator()(bool *p_open);
};

namespace glo {
class FboRenderer;
}

class FboWindow {
  using RenderFunc =
      std::function<void(int width, int height, std::chrono::nanoseconds time)>;

  std::string name_;
  std::shared_ptr<class glo::FboRenderer> fbo_;
  ImVec4 bg_ = {1, 1, 1, 1};
  ImVec4 tint_ = {1, 1, 1, 1};
  float clear_color_[4] = {0.3f, 0.2f, 0.1f, 1.0f};
  RenderFunc render_;
  std::chrono::nanoseconds time_;

  FboWindow(std::string_view name, const RenderFunc &render);

public:
  ~FboWindow();
  FboWindow(const FboWindow &) = delete;
  FboWindow &operator=(const FboWindow &) = delete;
  static std::shared_ptr<FboWindow> Create(std::string_view name,
                                           const RenderFunc &render);
  void show(bool *p_open);
  void update(std::chrono::nanoseconds time) { time_ = time; }

private:
  void render_fbo(float x, float y, float w, float h,
                  std::chrono::nanoseconds time);
};
