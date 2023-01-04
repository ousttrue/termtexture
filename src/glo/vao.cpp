#include "glo/vao.h"
#include "glo/scoped_binder.h"
#include "plog/Log.h"
#include <gl/glew.h>

namespace glo {

//
// VAO
//
VAO::VAO(const std::shared_ptr<::glo::VBO> &vbo) : vbo_(vbo) {
  glGenVertexArrays(1, &vao_);
}
VAO::~VAO() { glDeleteVertexArrays(1, &vao_); }
std::shared_ptr<VAO> VAO::Create(const std::shared_ptr<::glo::VBO> vbo,
                                 std::span<VertexLayout> layouts) {
  auto ptr = std::shared_ptr<VAO>(new VAO(vbo));

  auto vao_bind = ScopedBind(ptr);
  auto vbo_bind = ScopedBind(vbo);
  for (auto &layout : layouts) {
    glEnableVertexAttribArray(layout.attribute.location);
    switch (layout.gl_type) {
    case GL_FLOAT:
      glVertexAttribPointer(layout.attribute.location, layout.item_count,
                            GL_FLOAT, GL_FALSE, layout.stride,
                            (void *)(uint64_t)layout.byte_offset);
      break;

    case GL_UNSIGNED_BYTE:
      glVertexAttribPointer(layout.attribute.location, layout.item_count,
                            GL_UNSIGNED_BYTE, GL_TRUE, layout.stride,
                            (void *)(uint64_t)layout.byte_offset);
      break;

    default:
      PLOG_FATAL << "unknown gl_type";
      return nullptr;
    }
  }

  return ptr;
}
void VAO::Bind() { glBindVertexArray(vao_); }
void VAO::Unbind() { glBindVertexArray(0); }
void VAO::Draw(int topology, int offset, int count) {
  Bind();
  glDrawArrays(topology, offset, count);
  Unbind();
}

} // namespace glo