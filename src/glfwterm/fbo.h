#pragma once
#include "texture.h"

namespace glo {
class Fbo {
  glo::Texture texture_;
  uint32_t fbo_ = 0;
  uint32_t depth_ = 0;

public:
  Fbo(int width, int height, bool use_depth = true);
  ~Fbo();
  glo::Texture *Texture() { return &texture_; }
  void Bind();
  void Unbind();
};

} // namespace glo
