#pragma once
#include "texture.h"
#include <memory>

namespace glo {
class Fbo {
  std::shared_ptr<glo::Texture> texture_;
  uint32_t fbo_ = 0;
  uint32_t depth_ = 0;

public:
  Fbo(int width, int height, bool use_depth = true);
  ~Fbo();
  std::shared_ptr<glo::Texture> Texture() { return texture_; }
  void Bind();
  void Unbind();
};

class FboRenderer {
  std::shared_ptr<glo::Fbo> fbo_;

public:
  FboRenderer();
  ~FboRenderer();
  uint32_t Begin(int width, int height, const float color[4]);
  void End();
};

} // namespace glo
