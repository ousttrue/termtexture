#include "glo/ubo.h"
#include <GL/glew.h>

namespace glo {

UBO::UBO() { glCreateBuffers(1, &ubo_); }
UBO::~UBO() { glDeleteBuffers(1, &ubo_); }
std::shared_ptr<UBO> UBO::Create() { return std::shared_ptr<UBO>(new UBO); }
void UBO::Bind() { glBindBuffer(GL_UNIFORM_BUFFER, ubo_); }
void UBO::Unbind() { glBindBuffer(GL_UNIFORM_BUFFER, 0); }
void UBO::Upload(const void *data, uint32_t size) {
  Bind();
  glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
  Unbind();
}
void UBO::BindBase(int binding) {
  glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo_);
}

} // namespace glo
