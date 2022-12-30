#pragma once

namespace glo {

class Triangle {
  class TriangleImpl *impl_ = nullptr;

public:
  Triangle(const Triangle &) = delete;
  Triangle &operator=(const Triangle &) = delete;
  Triangle();
  ~Triangle();
  bool Load();
  void Render();
};

} // namespace scene
