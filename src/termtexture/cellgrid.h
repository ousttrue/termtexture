#include "vterm.h"
#include <chrono>
#include <memory>
#include <span>
#include <stdint.h>
#include <string>
#include <string_view>
#include <unordered_map>

struct CellPos {
  uint16_t row;
  uint16_t col;

  size_t value() const { return *((uint32_t *)this); }
  bool operator==(const CellPos &rhs) const { return value() == rhs.value(); }
};

template <> struct std::hash<CellPos> {
  std::size_t operator()(const CellPos &p) const noexcept { return p.value(); }
};

struct CellVertex {
  float col;
  float row;
  float glyph_index;
  uint8_t fg_color[4];
  uint8_t bg_color[4];
};

class CellGrid {
  int cell_width_ = 16;
  int cell_height_ = 16;
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
  bool Load(std::string_view path, int font_size, uint32_t atlas_size);
  void Clear();
  void SetCell(CellPos pos, const VTermScreenCell &cell);
  void PushText(const std::u32string &unicodes);
  void Commit();
  void Render(int width, int height, std::chrono::nanoseconds duration);
};
