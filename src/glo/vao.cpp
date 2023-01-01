#include "glo/vao.h"
#include "glo/scoped_binder.h"
#include <gl/glew.h>

namespace glo {

//
// VAO
//
VAO::VAO(const std::shared_ptr<VBO> &vbo) : vbo_(vbo) {
  glGenVertexArrays(1, &vao_);
}
VAO::~VAO() { glDeleteVertexArrays(1, &vao_); }
std::shared_ptr<VAO> VAO::Create(const std::shared_ptr<VBO> vbo,
                                 std::span<VertexLayout> layouts) {
  auto ptr = std::shared_ptr<VAO>(new VAO(vbo));

  auto vao_bind = ScopedBind(ptr);
  auto vbo_bind = ScopedBind(vbo);
  for (auto &layout : layouts) {
    glEnableVertexAttribArray(layout.attribute.location);
    glVertexAttribPointer(layout.attribute.location, layout.item_count,
                          GL_FLOAT, GL_FALSE, layout.stride,
                          (void *)(uint64_t)layout.byte_offset);
  }

  return ptr;
}
void VAO::Bind() { glBindVertexArray(vao_); }
void VAO::Unbind() { glBindVertexArray(0); }
void VAO::Draw(int topology, int offset, int count) {
  glDrawArrays(topology, offset, count);
}

} // namespace glo