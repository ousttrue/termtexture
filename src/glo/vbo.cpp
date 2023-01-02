#include "glo/vbo.h"
#include <GL/glew.h>
#include <assert.h>
#include <stdint.h>

namespace glo {

//
// VBO
//
VBO::VBO() { glGenBuffers(1, &vbo_); }
VBO::~VBO() { glDeleteBuffers(1, &vbo_); }
std::shared_ptr<VBO> VBO::Create() { return std::shared_ptr<VBO>(new VBO); }
void VBO::Bind() { glBindBuffer(GL_ARRAY_BUFFER, vbo_); }
void VBO::Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
void VBO::SetData(uint32_t buffer_size, const void *data, bool is_dynamic) {
  // assert(buffer_size);
  Bind();
  if (data) {
    glBufferData(GL_ARRAY_BUFFER, buffer_size, data, GL_STATIC_DRAW);
  } else {
    glBufferData(GL_ARRAY_BUFFER, buffer_size, data, GL_DYNAMIC_DRAW);
  }
  Unbind();
}
void VBO::SetSubData(const void *data, uint32_t offset, uint32_t size) {
  Bind();
  glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
  Unbind();
}

} // namespace glo
