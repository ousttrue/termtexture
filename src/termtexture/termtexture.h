#pragma once

#include "celltypes.h"
#include <chrono>
#include <memory>
#include <string_view>
#include <vterm.h>

namespace termtexture {

struct TermSize {
  int rows;
  int cols;
  bool operator==(const TermSize &rhs) const {
    return rows == rhs.rows && cols == rhs.cols;
  }
};

class TermTexture {
  class TermTextureImpl *impl_ = nullptr;
  TermTexture();

public:
  ~TermTexture();
  TermTexture(const TermTexture &) = delete;
  TermTexture &operator=(const TermTexture &) = delete;
  static std::shared_ptr<TermTexture> Create();
  TermSize TermSizeFromTextureSize(int width, int height) const;
  bool LoadFont(std::string_view fontfile, PixelSize cell_size);
  bool Launch(const char *cmd, TermSize size = {.rows = 24, .cols = 80});
  void Render(int width, int height, std::chrono::nanoseconds duration);
  void KeyboardUnichar(char c, VTermModifier mod);
  void KeyboardKey(VTermKey key, VTermModifier mod);
  bool IsClosed() const;
};

} // namespace termtexture
