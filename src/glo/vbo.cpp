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
  if (data && size) {
    ptr->Bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    ptr->Unbind();
  }
  return ptr;
}
void VBO::Bind() { glBindBuffer(GL_ARRAY_BUFFER, vbo_); }
void VBO::Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

} // namespace glo
