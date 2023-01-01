#include <chrono>
#include <memory>

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
  bool Load();
  void Render(int width, int height, std::chrono::nanoseconds duration);
};
} // namespace glo
