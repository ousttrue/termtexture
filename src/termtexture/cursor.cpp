#include "cursor.h"
#include <glo/shader.h>
#include <glo/texture.h>
#include <glo/ubo.h>
#include <glo/vao.h>
#include <memory>

struct CursorImpl {
  std::shared_ptr<glo::VAO> vao_;
  std::shared_ptr<glo::ShaderProgram> shader_;
  std::shared_ptr<glo::Texture> font_;

public:
  CursorImpl() {}
  void Render(const VTermPos &pos, PixelSize screen_size, PixelSize cell_size) {
  }
};

Cursor::Cursor() : impl_(new CursorImpl) {}
Cursor::~Cursor() { delete impl_; }
std::shared_ptr<Cursor> Cursor::Create() {
  return std::shared_ptr<Cursor>(new Cursor);
}
void Cursor::Render(const VTermPos &pos, PixelSize screen_size,
                    PixelSize cell_size) {
  impl_->Render(pos, screen_size, cell_size);
}
