#pragma once
#include <functional>
#include <imgui.h>
#include <memory>

void another_window(bool *p_open);

struct simple_window {
  bool *show_demo_window_;
  bool *show_another_window_;
  float *clear_color;
  void operator()(bool *p_open);
};

struct fbo_window {
  std::shared_ptr<class FboRenderer> fbo_;
  ImVec4 bg_ = {1, 1, 1, 1};
  ImVec4 tint_ = {1, 1, 1, 1};
  float clear_color_[4] = {0.3f, 0.2f, 0.1f, 1.0f};
  std::function<void()> render_;

  fbo_window(const std::function<void()> &render);
  void operator()(bool *p_open);

private:
  void show_fbo(float x, float y, float w, float h);
};
