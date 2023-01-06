#pragma once
#include <memory>
#include <span>
#include <stdint.h>
#include <string_view>
#include <unordered_map>
#include <vector>

struct GlyphRect {
  float x;
  float y;
  float w;
  float h;
};

struct GlyphOffset {
  float xoff;
  float yoff;
  float doublewidth;
  float expand;
};

struct Glyph {
  GlyphRect xywh;
  GlyphOffset offset;
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
  std::unordered_map<uint32_t, size_t> codepoint_map;

public:
  size_t GlyphIndexFromCodePoint(std::span<const uint32_t> codepoints);
  void Pack(uint8_t *atlas_bitmap, int atlas_width, int atlas_height,
            const FontLoader *font, std::span<GlyphPackRange> ranges);
};
