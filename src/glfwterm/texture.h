#pragma once
#include <stdint.h>

namespace glo {
class Texture {
  int width_;
  int height_;
  // GL_RGBA(32bit) or GL_RED(8bit graysclale)
  int pixel_type_;
  uint32_t handle_;

public:
  Texture(int width, int height, int pixel_type, const uint8_t *data = nullptr);
  ~Texture();
  uint32_t Handle() const { return handle_; }
  int Width() const { return width_; }
  int Height() const { return height_; }
  void Update(int x, int y, int w, int h, const uint8_t *data);
  void Bind();
  void Unbind();
};

} // namespace glo
