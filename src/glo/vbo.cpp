#include "glo/vbo.h"
#include <GL/glew.h>
#include <stdint.h>

namespace glo {

//
// VBO
//
VBO::VBO() { glGenBuffers(1, &vbo_); }
VBO::~VBO() { glDeleteBuffers(1, &vbo_); }
std::shared_ptr<VBO> VBO::Create(const void *data, uint32_t size) {
  auto ptr = std::shared_ptr<VBO>(new VBO);
  ptr->Bind();
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  ptr->Unbind();
  return ptr;
}
void VBO::Bind() { glBindBuffer(GL_ARRAY_BUFFER, vbo_); }
void VBO::Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

//
// VAO
//
VAO::VAO(const std::shared_ptr<VBO> &vbo) : vbo_(vbo) {
  glGenVertexArrays(1, &vao_);
}
VAO::~VAO() { glDeleteVertexArrays(1, &vao_); }
std::shared_ptr<VAO> VAO::Create(const std::shared_ptr<VBO> vbo) {
  return std::shared_ptr<VAO>(new VAO(vbo));
}
void VAO::Bind() { glBindVertexArray(vao_); }
void VAO::Unbind() { glBindVertexArray(0); }
void VAO::Draw(int topology, int offset, int count) {
  glDrawArrays(topology, offset, count);
}

} // namespace glo
