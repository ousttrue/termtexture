#include "termtexture.h"
#include "common_pty.h"
#include "text.h"
#include "vterm_object.h"
#include <memory>

namespace termtexture {

class TermTextureImpl {
  std::shared_ptr<glo::Text> text_;
  int cell_width_;
  int cell_height_;
  int rows_ = 24;
  int cols_ = 80;

  common_pty::Pty pty_;

public:
  std::shared_ptr<VTermObject> vterm_;
  TermTextureImpl() {
    text_ = glo::Text::Create();
    vterm_ = std::shared_ptr<VTermObject>(new VTermObject(
        rows_, cols_,
        [](const char *s, size_t len, void *user) {
          ((common_pty::Pty *)user)->Write(s, len);
        },
        &pty_));
  }

  bool LoadFont(std::string_view fontfile, int cell_width, int cell_height) {
    cell_width_ = cell_width;
    cell_height_ = cell_height;
    return text_->Load(fontfile, cell_height, 1024);
  }

  void Launch(const char *cmd) { pty_.Launch(rows_, cols_, cmd); }

  void Render(int width, int height, std::chrono::nanoseconds duration) {

    auto cols = width / cell_width_;
    auto rows = height / cell_height_;
    if (rows != rows_ || cols != cols_) {
      // resize
      rows_ = rows;
      cols_ = cols;
      pty_.NotifyTermSize(rows, cols);
      vterm_->set_rows_cols(rows, cols);
    }

    // pty to vterm
    auto input = pty_.Read();
    if (!input.empty()) {
      vterm_->input_write(input.data(), input.size());
    }

    // vterm to screen
    bool ringing;
    auto &damaged = vterm_->new_frame(&ringing, true);
    if (!damaged.empty()) {
      for (auto &pos : damaged) {
        if (auto cell = vterm_->get_cell(pos)) {
          int i = 0;
          for (; i < VTERM_MAX_CHARS_PER_CELL && cell->chars[i]; ++i) {
          }
          std::span<uint32_t> span(cell->chars, i);
          text_->SetCell(
              {
                  .row = (uint16_t)pos.row,
                  .col = (uint16_t)pos.col,
              },
              span);
        }
      }
      text_->Commit();
    }

    text_->Render(width, height, duration);
  }
};

TermTexture::TermTexture() : impl_(new TermTextureImpl) {}

TermTexture::~TermTexture() { delete impl_; }

std::shared_ptr<TermTexture> TermTexture::Create() {
  return std::shared_ptr<TermTexture>(new TermTexture);
}

bool TermTexture::LoadFont(std::string_view fontfile, int cell_width,
                           int cell_height) {
  return impl_->LoadFont(fontfile, cell_width, cell_height);
}

bool TermTexture::Launch(const char *cmd) {
  impl_->Launch(cmd);
  return true;
}

void TermTexture::Render(int width, int height,
                         std::chrono::nanoseconds duration) {
  impl_->Render(width, height, duration);
}

void TermTexture::KeyboardUnichar(char c, VTermModifier mod) {
  impl_->vterm_->keyboard_unichar(c, mod);
}

void TermTexture::KeyboardKey(VTermKey key, VTermModifier mod) {
  impl_->vterm_->keyboard_key(key, mod);
}

} // namespace termtexture
