#include "celltypes.h"
#include "vterm.h"
#include <chrono>
#include <memory>
#include <span>
#include <stdint.h>
#include <string>
#include <string_view>
#include <unordered_map>

struct CellVertex {
  float col;
  float row;
  float glyph_index;
  uint8_t fg_color[4];
  uint8_t bg_color[4];
};

/// Windows, Texture, Screen: PixelSize
/// Term: TermSize(rows, cols)
/// Cell: CellSize(rows, cols)
class CellGrid {
  PixelSize cell_size_ = {
      .width = 8,
      .height = 16,
  };
  std::vector<CellVertex> cells_;
  std::unordered_map<CellPos, size_t, std::hash<CellPos>> cellMap_;
  class TextImpl *impl_ = nullptr;

public:
  CellGrid();

public:
  ~CellGrid();
  static std::shared_ptr<CellGrid> Create();
  CellGrid(const CellGrid &) = delete;
  CellGrid &operator=(const CellGrid &) = delete;
  PixelSize CellSize() const { return cell_size_; }
  bool Load(std::string_view path, PixelSize cell_size, uint32_t atlas_size);
  void Clear();
  void SetCell(CellPos pos, const VTermScreenCell &cell);
  void PushText(const std::u32string &unicodes);
  void Commit();
  void Render(PixelSize screen_size, std::chrono::nanoseconds duration);
};
