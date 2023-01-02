#include <chrono>
#include <memory>
#include <span>
#include <stdint.h>
#include <string>
#include <string_view>

namespace glo {
struct Cell {
  uint16_t row;
  uint16_t col;

  size_t value() const { return *((uint32_t *)this); }
  bool operator==(const Cell &rhs) const { return value() == rhs.value(); }
};

class Text {
public:
  class TextImpl *impl_ = nullptr;

  Text();

public:
  ~Text();
  static std::shared_ptr<Text> Create();
  Text(const Text &) = delete;
  Text &operator=(const Text &) = delete;
  bool Load(const std::string &path, float font_size, uint32_t atlas_size);
  void Clear();
  void SetCell(Cell cell, std::span<uint32_t> codepoints);
  void PushText(const std::u32string &unicodes);
  void Commit();
  void Render(int width, int height, std::chrono::nanoseconds duration);
};
} // namespace glo
