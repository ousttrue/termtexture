#pragma once
#include <span>
#include <stdint.h>
#include <string_view>
#include <vector>

const auto GLYPH_COUNT = 95;

struct Glyph {
  float xywh[4];
  float offset[4];
};

struct FontAtlas {

  Glyph glyphs[GLYPH_COUNT];
  std::vector<uint8_t> bitmap;
  float ascents = 0;
  float descents = 0;
  float linegaps = 0;

  size_t GlyphIndexFromCodePoint(std::span<uint32_t> codepoints);

  // https://gist.github.com/vassvik/f442a4cc6127bc7967c583a12b148ac9
  const std::vector<uint8_t> &LoadFont(std::string_view path, float font_size,
                                       uint32_t atlas_size);
};
