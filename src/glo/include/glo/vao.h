#pragma once
#include "vbo.h"
#include <memory>
#include <span>
#include <string>

namespace glo {

struct AttributeLocation {
  std::string name;
  uint32_t location;
};

struct VertexLayout {
  AttributeLocation attribute;
  // maybe float1, 2, 3, 4 and 16
  uint32_t item_count;
  uint32_t stride;
  uint32_t byte_offset;
};

class VAO {
  uint32_t vao_ = 0;
  std::shared_ptr<VBO> vbo_;
  VAO(const std::shared_ptr<VBO> &vbo);

public:
  ~VAO();
  VAO(const VAO &) = delete;
  VAO &operator=(const VAO &) = delete;
  static std::shared_ptr<VAO> Create(const std::shared_ptr<VBO> vbo,
                                     std::span<VertexLayout> layouts);
  std::shared_ptr<VBO> GetVBO() { return vbo_; }
  void Bind();
  void Unbind();
  void Draw(int topology, int offset, int count);
};

} // namespace glo