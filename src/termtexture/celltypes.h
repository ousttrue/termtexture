#pragma once
#include <stdint.h>

struct CellSize {
  uint16_t pixel_width;
  uint16_t pixel_height;

  static CellSize FromFontHeight(int font_pixel_height) {
    return {
        .pixel_width = static_cast<uint16_t>(font_pixel_height / 2),
        .pixel_height = static_cast<uint16_t>(font_pixel_height),
    };
  }
};
