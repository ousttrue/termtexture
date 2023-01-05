#include "fontatlas.h"
#include "readallbytes.h"
#include <gl/glew.h>
#include <memory>
#include <plog/Log.h>
#include <stdint.h>
#include <string_view>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb_truetype.h>

bool FontLoader::Load(std::string_view path, float fontsize) {
  ttf = ReadAllBytes<unsigned char>(path);
  if (ttf.empty()) {
    return false;
  }
  info.fontsize = fontsize;

  // get the global metrics for each size/range
  {
    stbtt_fontinfo stb_info;
    stbtt_InitFont(&stb_info, ttf.data(),
                   stbtt_GetFontOffsetForIndex(ttf.data(), 0));

    auto scale = stbtt_ScaleForPixelHeight(&stb_info, fontsize);
    int a;
    int d;
    int l;
    stbtt_GetFontVMetrics(&stb_info, &a, &d, &l);
    info.ascents = a * scale;
    info.descents = d * scale;
    info.linegaps = l * scale;
  }

  return true;
}

size_t
FontAtlas::GlyphIndexFromCodePoint(std::span<const uint32_t> codepoints) {
  if (codepoints.empty()) {
    return 0;
  }
  auto codepoint = codepoints[0];
  if (codepoint < 32) {
    return 0;
  }
  return codepoint - 32;
}

void FontAtlas::Pack(uint8_t *atlas_bitmap, int atlas_width, int atlas_height,
                     const FontLoader *font, std::span<GlyphPackRange> ranges) {

  std::vector<std::vector<stbtt_packedchar>> tmp_buffer;
  for (auto &range : ranges) {
    tmp_buffer.push_back(std::vector<stbtt_packedchar>(range.length));
  }
  std::vector<stbtt_pack_range> stb_ranges;
  for (int i = 0; i < ranges.size(); ++i) {
    stb_ranges.push_back({
        .font_size = info.fontsize,
        .first_unicode_codepoint_in_range = (int)ranges[i].codepoint,
        .num_chars = (int)ranges[i].length,
        .chardata_for_range = tmp_buffer[i].data(),
    });
  }

  stbtt_pack_context pc;
  stbtt_PackBegin(&pc, atlas_bitmap, atlas_width, atlas_height, 0, 1, nullptr);
  // say, choose 3x1 oversampling for subpixel positioning
  stbtt_PackSetOversampling(&pc, 1, 1);
  stbtt_PackFontRanges(&pc, font->ttf.data(), 0, stb_ranges.data(),
                       stb_ranges.size());
  stbtt_PackEnd(&pc);

  for (auto &stb_range : stb_ranges) {
    for (auto &g :
         std::span{stb_range.chardata_for_range, (size_t)stb_range.num_chars}) {
      // auto &g = packed[i];
      glyphs.push_back({
          .xywh = {(float)g.x0, (float)g.y0, (float)g.x1, (float)g.y1},
          .offset = {(float)g.xoff, (float)g.yoff, (float)g.xoff2,
                     (float)g.yoff2},
      });
    }
  }
}
