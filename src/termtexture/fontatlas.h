#pragma once
#include <memory>
#include <span>
#include <stdint.h>
#include <string_view>
#include <vector>

struct Glyph {
  float xywh[4];
  float offset[4];
};

struct FontInfo {
  float fontsize = 0;
  float ascents = 0;
  float descents = 0;
  float linegaps = 0;
};

struct FontLoader {
  std::vector<uint8_t> ttf;
  FontInfo info;

  bool Load(std::string_view path, float fontsize);
};

struct GlyphPackRange {
  uint32_t codepoint;
  uint32_t length;
};

struct FontAtlas {
  std::vector<Glyph> glyphs;
  FontInfo info;

public:
  size_t GlyphIndexFromCodePoint(std::span<const uint32_t> codepoints);
  void Pack(uint8_t *atlas_bitmap, int atlas_width, int atlas_height,
            const FontLoader *font, std::span<GlyphPackRange> ranges);
};
