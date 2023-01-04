#pragma once

#include <chrono>
#include <memory>
#include <string_view>
#include <vterm.h>

namespace termtexture {

class TermTexture {
  class TermTextureImpl *impl_ = nullptr;
  TermTexture();

public:
  ~TermTexture();
  TermTexture(const TermTexture &) = delete;
  TermTexture &operator=(const TermTexture &) = delete;
  static std::shared_ptr<TermTexture> Create();
  bool LoadFont(std::string_view fontfile, int cell_width, int cell_height);
  bool Launch(const char *cmd);
  void Render(int width, int height, std::chrono::nanoseconds duration);
  void KeyboardUnichar(char c, VTermModifier mod);
  void KeyboardKey(VTermKey key, VTermModifier mod);
  bool IsClosed() const;
};

} // namespace termtexture
