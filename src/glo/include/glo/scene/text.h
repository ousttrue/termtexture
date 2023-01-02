#include <chrono>
#include <memory>
#include <string>
#include <string_view>

namespace glo {
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
  void PushText(const std::u32string &unicodes);
  void Render(int width, int height, std::chrono::nanoseconds duration);
};
} // namespace glo
