#include "fontatlas.h"
#include "readallbytes.h"
#include <gl/glew.h>
#include <plog/Log.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb_truetype.h>

size_t FontAtlas::GlyphIndexFromCodePoint(std::span<const uint32_t> codepoints) {
  if (codepoints.empty()) {
    return 0;
  }
  auto codepoint = codepoints[0];
  if (codepoint < 32) {
    return 0;
  }
  return codepoint - 32;
}

// https://gist.github.com/vassvik/f442a4cc6127bc7967c583a12b148ac9
const std::vector<uint8_t> &FontAtlas::LoadFont(std::string_view path,
                                                float font_size,
                                                uint32_t atlas_size) {
  auto ttf_buffer = ReadAllBytes<unsigned char>(path);
  bitmap.clear();
  if (ttf_buffer.empty()) {
    return bitmap;
  }
  PLOG_INFO << path << std::endl;

  {
    stbtt_packedchar packed[GLYPH_COUNT];
    stbtt_pack_range range{
        .font_size = font_size,
        .first_unicode_codepoint_in_range = 32,
        .num_chars = GLYPH_COUNT,
        .chardata_for_range = packed,
    };

    // make a most likely large enough bitmap, adjust to font type, number of
    // sizes and glyphs and oversampling
    auto width = atlas_size;
    auto max_height = atlas_size;
    bitmap.resize(width * max_height);
    assert(bitmap.size());
    // do the packing, based on the ranges specified
    stbtt_pack_context pc;
    stbtt_PackBegin(&pc, bitmap.data(), width, max_height, 0, 1, nullptr);
    // say, choose 3x1 oversampling for subpixel positioning
    stbtt_PackSetOversampling(&pc, 1, 1);
    stbtt_PackFontRanges(&pc, ttf_buffer.data(), 0, &range, 1);
    stbtt_PackEnd(&pc);

    for (int i = 0; i < GLYPH_COUNT; ++i) {
      auto &g = packed[i];
      glyphs[i] = {
          .xywh = {(float)g.x0, (float)g.y0, (float)g.x1, (float)g.y1},
          .offset = {(float)g.xoff, (float)g.yoff, (float)g.xoff2,
                     (float)g.yoff2},
      };
    }
  }

  // get the global metrics for each size/range
  {
    stbtt_fontinfo info;
    stbtt_InitFont(&info, ttf_buffer.data(),
                   stbtt_GetFontOffsetForIndex(ttf_buffer.data(), 0));

    auto scale = stbtt_ScaleForPixelHeight(&info, font_size);
    int a;
    int d;
    int l;
    stbtt_GetFontVMetrics(&info, &a, &d, &l);
    ascents = a * scale;
    descents = d * scale;
    linegaps = l * scale;
  }

  return bitmap;
}
