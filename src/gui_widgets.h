#pragma once

void another_window(bool *show_another_window_);

struct simple_window {
  bool *show_demo_window_;
  bool *show_another_window_;
  float *clear_color;
  void operator()(bool *show);
};
