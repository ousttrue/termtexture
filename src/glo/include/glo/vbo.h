#pragma once
#include <memory>
#include <stdint.h>

namespace glo {
class VBO {
  uint32_t vbo_ = 0;

  VBO();

public:
  VBO(const VBO &) = delete;
  VBO &operator=(const VBO &) = delete;
  ~VBO();
  static std::shared_ptr<VBO> Create(const void *data, uint32_t size);
  static std::shared_ptr<VBO> Create()
  {
    return Create(0, 0);
  }
  void Bind();
  void Unbind();
};

} // namespace glo
