#pragma once
#include <chrono>
#include <memory>

namespace glo {

class Triangle {
  class TriangleImpl *impl_ = nullptr;

  Triangle();

public:
  ~Triangle();
  static std::shared_ptr<Triangle> Create();
  Triangle(const Triangle &) = delete;
  Triangle &operator=(const Triangle &) = delete;
  bool Load();
  void Render(int width, int height, std::chrono::nanoseconds duration);
};

} // namespace glo
