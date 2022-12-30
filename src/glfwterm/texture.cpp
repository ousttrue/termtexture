#include "texture.h"
#include <GL/glew.h>

namespace glo {
Texture::Texture(int width, int height, int pixel_type, const uint8_t *data)
    : width_(width), height_(height), pixel_type_(pixel_type) {
  glGenTextures(1, &handle_);
  // logger.debug(f'Texture: {self.handle}')

  Bind();

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  // glPixelStorei(GL_UNPACK_ROW_LENGTH, width)
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

  glTexImage2D(GL_TEXTURE_2D, 0, pixel_type_, width_, height_, 0, pixel_type_,
               GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  Unbind();
}

Texture::~Texture() {
  // logger.debug(f'{self.handle}')
  glDeleteTextures(1, &handle_);
}

void Texture::Update(int x, int y, int w, int h, const uint8_t *data) {
  Bind();

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, width_);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, y);

  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, pixel_type_, GL_UNSIGNED_BYTE,
                  data);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

  Unbind();
}

void Texture::Bind() { glBindTexture(GL_TEXTURE_2D, handle_); }

void Texture::Unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

} // namespace glo
