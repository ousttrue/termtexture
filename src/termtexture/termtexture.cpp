#include "termtexture.h"
#include "cellgrid.h"
#include "common_pty.h"
#include "vterm_object.h"
#include <memory>

namespace termtexture {

class TermTextureImpl {
  std::shared_ptr<CellGrid> grid_;
  int cell_width_ = 0;
  int cell_height_ = 0;
  int rows_ = 24;
  int cols_ = 80;
  std::string lazy_launch_;

public:
  common_pty::Pty pty_;
  std::shared_ptr<VTermObject> vterm_;
  TermTextureImpl() {
    grid_ = CellGrid::Create();
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
    return grid_->Load(fontfile, cell_height, 1024);
  }

  void Launch(const char *cmd) { lazy_launch_ = cmd; }

  void Render(int width, int height, std::chrono::nanoseconds duration) {

    auto cols = width / cell_width_;
    auto rows = height / cell_height_;
    if (rows != rows_ || cols != cols_) {
      // resize
      rows_ = rows;
      cols_ = cols;
      if (!lazy_launch_.empty()) {
        pty_.Launch(rows_, cols_, lazy_launch_.c_str());
        lazy_launch_ = {};
      }
      pty_.NotifyTermSize(rows, cols);
      vterm_->resize_rows_cols(rows, cols);
      grid_->Clear();
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
          grid_->SetCell(
              {
                  .row = (uint16_t)pos.row,
                  .col = (uint16_t)pos.col,
              },
              *cell);
        }
      }
      grid_->Commit();
    }

    grid_->Render(width, height, duration);

    if (auto cursor = vterm_->get_cursor()) {
      // cursor_->Render(cursor);
    }
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

bool TermTexture::IsClosed() const { return impl_->pty_.IsClosed(); }

} // namespace termtexture
