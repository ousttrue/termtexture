#include "fbo.h"
#include <GL/glew.h>

namespace glo {

Fbo::Fbo(int width, int height, bool use_depth)
    : texture_(width, height, GL_RGBA) {
  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         texture_.Handle(), 0);
  unsigned int buf = GL_COLOR_ATTACHMENT0;
  glDrawBuffers(1, &buf);

  if (use_depth) {
    glGenRenderbuffers(1, &depth_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, depth_);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // LOGGER.debug(f'fbo: {self.fbo}, texture: {self.texture}, depth:
  // {self.depth}')
}

Fbo::~Fbo() {
  // LOGGER.debug(f'fbo: {self.fbo}')
  glDeleteFramebuffers(1, &fbo_);
}

void Fbo::Bind() { glBindFramebuffer(GL_FRAMEBUFFER, fbo_); }

void Fbo::Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

} // namespace glo
