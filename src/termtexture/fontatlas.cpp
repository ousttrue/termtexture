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

class FontPacker {
  const FontLoader *font_ = nullptr;
  uint8_t *atlas_;
  int atlas_width_;
  int atlas_height_;
  std::vector<stbtt_packedchar> packed_;

  FontPacker(const FontLoader *font, uint8_t *atlas, int atlas_width,
             int atlas_height)
      : font_(font), atlas_(atlas), atlas_width_(atlas_width),
        atlas_height_(atlas_height) {}

public:
  static std::shared_ptr<FontPacker> Create(const FontLoader *font,
                                            uint8_t *atlas, int atlas_width,
                                            int atlas_height) {
    return std::shared_ptr<FontPacker>(
        new FontPacker(font, atlas, atlas_width, atlas_height));
  }

  const stbtt_packedchar *Pack(int begin, int count, float font_size) {
    {
      packed_.resize(count);
      stbtt_pack_range range{
          .font_size = font_size,
          .first_unicode_codepoint_in_range = begin,
          .num_chars = (int)packed_.size(),
          .chardata_for_range = packed_.data(),
      };

      stbtt_pack_context pc;
      stbtt_PackBegin(&pc, atlas_, atlas_width_, atlas_height_, 0, 1,
                      nullptr);
      // say, choose 3x1 oversampling for subpixel positioning
      stbtt_PackSetOversampling(&pc, 1, 1);
      stbtt_PackFontRanges(&pc, font_->ttf.data(), 0, &range, 1);
      stbtt_PackEnd(&pc);

      return packed_.data();
    }
  }
};

void FontAtlas::Pack(uint8_t *atlas_bitmap, int atlas_width, int atlas_height,
                     const FontLoader *font, uint32_t codepoint, int count) {

  info = font->info;

  auto packer =
      FontPacker::Create(font, atlas_bitmap, atlas_width, atlas_height);

  auto packed = packer->Pack(codepoint, count, info.fontsize);
  for (int i = 0; i < count; ++i) {
    auto &g = packed[i];
    glyphs.push_back({
        .xywh = {(float)g.x0, (float)g.y0, (float)g.x1, (float)g.y1},
        .offset = {(float)g.xoff, (float)g.yoff, (float)g.xoff2,
                   (float)g.yoff2},
    });
  }
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
