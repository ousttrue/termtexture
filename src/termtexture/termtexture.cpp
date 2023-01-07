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
  TermSize size_ = {
      .rows = 24,
      .cols = 80,
  };

public:
  common_pty::Pty pty_;
  std::shared_ptr<VTermObject> vterm_;
  TermTextureImpl() {
    grid_ = CellGrid::Create();
    vterm_ = std::shared_ptr<VTermObject>(new VTermObject(
        size_.rows, size_.cols,
        [](const char *s, size_t len, void *user) {
          ((common_pty::Pty *)user)->Write(s, len);
        },
        &pty_));
  }

  TermSize TermSizeFromTextureSize(int width, int height) const {
    auto cols = std::max(1, width / cell_width_);
    auto rows = std::max(1, height / cell_height_);
    return {.rows = rows, .cols = cols};
  }

  void UpdateTextureSize(int width, int height) {
    auto size = TermSizeFromTextureSize(width, height);
    if (size == size_) {
      return;
    }
    // resize
    size = size_;
    pty_.NotifyTermSize(size_.rows, size_.cols);
    vterm_->resize_rows_cols(size_.rows, size_.cols);
    grid_->Clear();
  }

  bool LoadFont(std::string_view fontfile, int cell_width, int cell_height) {
    cell_width_ = cell_width;
    cell_height_ = cell_height;
    return grid_->Load(fontfile, cell_height, 1024);
  }

  void Launch(TermSize size, const char *cmd) {
    size_ = size;
    vterm_->resize_rows_cols(size_.rows, size_.cols);
    pty_.Launch(size_.rows, size_.cols, cmd);
  }

  void Render(int width, int height, std::chrono::nanoseconds duration) {

    UpdateTextureSize(width, height);

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

TermSize TermTexture::TermSizeFromTextureSize(int width, int height) const {
  return impl_->TermSizeFromTextureSize(width, height);
}

bool TermTexture::LoadFont(std::string_view fontfile, int cell_width,
                           int cell_height) {
  return impl_->LoadFont(fontfile, cell_width, cell_height);
}

bool TermTexture::Launch(const char *cmd, TermSize size) {
  impl_->Launch(size, cmd);
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
