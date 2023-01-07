#include "termtexture.h"
#include "cellgrid.h"
#include "celltypes.h"
#include "common_pty.h"
#include "cursor.h"
#include "vterm_object.h"
#include <memory>

namespace termtexture {

class TermTextureImpl {
  std::shared_ptr<CellGrid> grid_;
  PixelSize cell_size_ = {
      0,
      0,
  };
  TermSize size_ = {
      .rows = 24,
      .cols = 80,
  };
  std::shared_ptr<Cursor> cursor_;

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
    cursor_ = Cursor::Create();
  }

  TermSize TermSizeFromTextureSize(PixelSize screen_size) const {
    auto cols = std::max(1, screen_size.width / cell_size_.width);
    auto rows = std::max(1, screen_size.height / cell_size_.height);
    return {.rows = rows, .cols = cols};
  }

  void UpdateTextureSize(PixelSize screen_size) {
    auto size = TermSizeFromTextureSize(screen_size);
    if (size == size_) {
      return;
    }
    // resize
    size_ = size;
    pty_.NotifyTermSize(size_.rows, size_.cols);
    vterm_->resize_rows_cols(size_.rows, size_.cols);
    grid_->Clear();
  }

  bool LoadFont(std::string_view fontfile, PixelSize cell_size) {
    cell_size_ = cell_size;
    return grid_->Load(fontfile, cell_size, 1024);
  }

  void Launch(TermSize size, const char *cmd) {
    size_ = size;
    vterm_->resize_rows_cols(size_.rows, size_.cols);
    pty_.Launch(size_.rows, size_.cols, cmd);
  }

  void Render(PixelSize size, std::chrono::nanoseconds duration) {

    UpdateTextureSize(size);

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

    grid_->Render(size, duration);

    if (auto cursor = vterm_->get_cursor()) {
      cursor_->Render(cursor.value(), size, grid_->CellSize());
    }
  }
};

TermTexture::TermTexture() : impl_(new TermTextureImpl) {}

TermTexture::~TermTexture() { delete impl_; }

std::shared_ptr<TermTexture> TermTexture::Create() {
  return std::shared_ptr<TermTexture>(new TermTexture);
}

TermSize TermTexture::TermSizeFromTextureSize(int width, int height) const {
  return impl_->TermSizeFromTextureSize({
      .width = static_cast<uint16_t>(width),
      .height = static_cast<uint16_t>(height),
  });
}

bool TermTexture::LoadFont(std::string_view fontfile, PixelSize cell_size) {
  return impl_->LoadFont(fontfile, cell_size);
}

bool TermTexture::Launch(const char *cmd, TermSize size) {
  impl_->Launch(size, cmd);
  return true;
}

void TermTexture::Render(int width, int height,
                         std::chrono::nanoseconds duration) {
  impl_->Render(
      {
          .width = static_cast<uint16_t>(width),
          .height = static_cast<uint16_t>(height),
      },
      duration);
}

void TermTexture::KeyboardUnichar(char c, VTermModifier mod) {
  impl_->vterm_->keyboard_unichar(c, mod);
}

void TermTexture::KeyboardKey(VTermKey key, VTermModifier mod) {
  impl_->vterm_->keyboard_key(key, mod);
}

bool TermTexture::IsClosed() const { return impl_->pty_.IsClosed(); }

} // namespace termtexture
