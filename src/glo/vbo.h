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
  void Bind();
  void Unbind();
};

class VAO {
  uint32_t vao_ = 0;
  std::shared_ptr<VBO> vbo_;
  VAO(const std::shared_ptr<VBO> &vbo);

public:
  ~VAO();
  VAO(const VAO &) = delete;
  VAO &operator=(const VAO &) = delete;
  static std::shared_ptr<VAO> Create(const std::shared_ptr<VBO> vbo);
  void Bind();
  void Unbind();
  void Draw(int topology, int offset, int count);
};

} // namespace glo
