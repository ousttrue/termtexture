#pragma once
#include <chrono>

namespace glo {

class Triangle {
  class TriangleImpl *impl_ = nullptr;

public:
  Triangle(const Triangle &) = delete;
  Triangle &operator=(const Triangle &) = delete;
  Triangle();
  ~Triangle();
  bool Load();
  void Render(int width, int height, std::chrono::nanoseconds duration);
};

} // namespace glo
