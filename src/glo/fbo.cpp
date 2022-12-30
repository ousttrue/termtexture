#include "fbo.h"
#include <GL/glew.h>

namespace glo {

//
// Fbo
//
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

//
// FboRenderer
//
FboRenderer::FboRenderer() {}
FboRenderer::~FboRenderer() {}
GLuint FboRenderer::Begin(int width, int height, const float color[4]) {
  if (width == 0 || height == 0) {
    return 0;
  }

  if (fbo_) {
    if (fbo_->Texture()->Width() != width ||
        fbo_->Texture()->Height() != height) {
      fbo_ = nullptr;
    }
  }
  if (!fbo_) {
    fbo_ = std::make_shared<glo::Fbo>(width, height);
  }

  fbo_->Bind();
  glViewport(0, 0, width, height);
  glScissor(0, 0, width, height);
  glClearColor(color[0] * color[3], color[1] * color[3], color[2] * color[3],
               color[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearDepth(1.0);
  glDepthFunc(GL_LESS);
  return fbo_->Texture()->Handle();
}
void FboRenderer::End() { fbo_->Unbind(); }

} // namespace glo
