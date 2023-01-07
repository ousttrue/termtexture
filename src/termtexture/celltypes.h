#pragma once
#include <stdint.h>
#include <type_traits>

struct PixelSize {
  uint16_t width;
  uint16_t height;

  static PixelSize CellSizeFromFontHeight(int font_pixel_height) {
    return {
        .width = static_cast<uint16_t>(font_pixel_height / 2),
        .height = static_cast<uint16_t>(font_pixel_height),
    };
  }
};

struct CellPos {
  uint16_t row;
  uint16_t col;

  size_t value() const { return *((uint32_t *)this); }
  bool operator==(const CellPos &rhs) const { return value() == rhs.value(); }
};

template <> struct std::hash<CellPos> {
  std::size_t operator()(const CellPos &p) const noexcept { return p.value(); }
};
