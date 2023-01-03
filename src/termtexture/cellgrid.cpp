#include "cellgrid.h"
#include "readallbytes.h"
#include <chrono>
#include <gl/glew.h>
#include <glo/scoped_binder.h>
#include <glo/shader.h>
#include <glo/texture.h>
#include <glo/ubo.h>
#include <glo/vao.h>
#include <ios>
#include <memory>
#include <plog/Log.h>
#include <stdint.h>
#include <string>
#include <type_traits>
#include <unordered_map>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb_truetype.h>

auto vs_src = R"(#version 420
in vec3 i_Pos;
in vec3 i_Color;
// out vec3 v_Color;
out vData { vec3 color; }
vertex;
void main() {
  gl_Position = vec4(i_Pos, 1);
  vertex.color = i_Color;
}
)";

auto gs_src = R"(#version 420 core
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

layout(std140, binding = 0) uniform Global {
  mat4 projection;
  vec2 screenSize;
  vec2 cellSize;
  vec2 atlasSize;
  float ascent;
  float descent;
}
global;

struct Glyph {
  vec4 xywh;
  vec4 offset;
};

layout(std140, binding = 1) uniform Glyphs { Glyph glyphs[128]; };

in vData { vec3 color; }
vertices[];
out vec2 g_TexCoords;
out vec3 g_Color;

vec2 pixelToUv(float x, float y) {
  return vec2((x + 0.5) / global.atlasSize.x, (y + 0.5) / global.atlasSize.y);
}

// -1+1 +1+1
//  0+----+2
//   |   /|
//   |  / |
//   | /  |
//   |/   |
//  1+----+3
// -1-1 +1-1
void main() {
  vec2 cellSize = global.cellSize;
  vec2 topLeft = gl_in[0].gl_Position.xy * cellSize + vec2(0, global.ascent);
  int glyphIndex = int(gl_in[0].gl_Position.z);
  Glyph glyph = glyphs[glyphIndex];
  float l = glyph.xywh.x;
  float t = glyph.xywh.y;
  float r = glyph.xywh.z;
  float b = glyph.xywh.w;

  // 0
  gl_Position = global.projection * vec4(topLeft + glyph.offset.xy, 0, 1);
  g_TexCoords = pixelToUv(l, t);
  g_Color = vertices[0].color;
  EmitVertex();

  // 1
  gl_Position = global.projection *
                vec4(topLeft + glyph.offset.xy + vec2(0, b - t), 0.0, 1);
  g_TexCoords = pixelToUv(l, b);
  g_Color = vertices[0].color;
  EmitVertex();

  // 2
  gl_Position = global.projection *
                vec4(topLeft + glyph.offset.xy + vec2(r - l, 0), 0.0, 1);
  g_TexCoords = pixelToUv(r, t);
  g_Color = vertices[0].color;
  EmitVertex();

  // 3
  gl_Position = global.projection *
                vec4(topLeft + glyph.offset.xy + vec2(r - l, b - t), 0.0, 1);
  g_TexCoords = pixelToUv(r, b);
  g_Color = vertices[0].color;
  EmitVertex();

  EndPrimitive();
}
)";

auto fs_src = R"(#version 460 core

in vec2 g_TexCoords;
in vec3 g_Color;
layout(location = 0) out vec4 FragColor;
uniform sampler2D uTex;

void main() {
  vec4 texcel = texture(uTex, g_TexCoords);
  FragColor = vec4(g_Color, texcel.x);
  // FragColor = vec4(TexCoords, 0, 1);
}
)";

const auto GLYPH_COUNT = 95;

struct FontAtlas {

  float ascents[GLYPH_COUNT];
  float descents[GLYPH_COUNT];
  float linegaps[GLYPH_COUNT];
  stbtt_packedchar glyphs[GLYPH_COUNT];

  size_t GlyphIndexFromCodePoint(std::span<uint32_t> codepoints) {
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
  std::shared_ptr<glo::Texture> LoadFont(std::string_view path, float font_size,
                                         uint32_t atlas_size) {
    auto ttf_buffer = ReadAllBytes<unsigned char>(path);
    if (ttf_buffer.empty()) {
      return {};
    }
    PLOG_INFO << path << std::endl;

    stbtt_pack_range range{
        .font_size = font_size,
        .first_unicode_codepoint_in_range = 32,
        .num_chars = GLYPH_COUNT,
        .chardata_for_range = glyphs,
    };

    // make a most likely large enough bitmap, adjust to font type, number of
    // sizes and glyphs and oversampling
    auto width = atlas_size;
    auto max_height = atlas_size;
    std::vector<uint8_t> bitmap(width * max_height);
    assert(bitmap.size());
    // do the packing, based on the ranges specified
    stbtt_pack_context pc;
    stbtt_PackBegin(&pc, bitmap.data(), width, max_height, 0, 1, nullptr);
    // say, choose 3x1 oversampling for subpixel positioning
    stbtt_PackSetOversampling(&pc, 1, 1);
    stbtt_PackFontRanges(&pc, ttf_buffer.data(), 0, &range, 1);
    stbtt_PackEnd(&pc);

    // get the global metrics for each size/range
    stbtt_fontinfo info;
    stbtt_InitFont(&info, ttf_buffer.data(),
                   stbtt_GetFontOffsetForIndex(ttf_buffer.data(), 0));

    {
      auto scale = stbtt_ScaleForPixelHeight(&info, range.font_size);
      int a;
      int d;
      int l;
      stbtt_GetFontVMetrics(&info, &a, &d, &l);
      int i = 0;
      ascents[i] = a * scale;
      descents[i] = d * scale;
      linegaps[i] = l * scale;
    }

    // {
    //   int j = 0;
    //   PLOG_DEBUG << "size   : " << range.font_size;
    //   PLOG_DEBUG << "ascent : " << ascents[j];
    //   PLOG_DEBUG << "descent: " << descents[j];
    //   PLOG_DEBUG << "linegap: " << linegaps[j];
    //   for (int i = 0; i < GLYPH_COUNT; ++i) {
    //                 PLOG_DEBUG << "    '{}':  (x0,y0) = ({},{}),  (x1,y1)
    //     //     =
    //     //     ({},{}),  (xoff,yoff) = ({},{}),  (xoff2,yoff2) = ({},{}),
    //     //     xadvance
    //     //     =
    //     //     {}", .{
    //     //     //             32 + i,
    //     //     //             g.x0,
    //     //     //             g.y0,
    //     //     //             g.x1,
    //     //     //             g.y1,
    //     //     //             g.xoff,
    //     //     //             g.yoff,
    //     //     //             g.xoff2,
    //     //     //             g.yoff2,
    //     //     //             g.xadvance,
    //     //     //         });
    //   }
    // }

    auto texture =
        glo::Texture::Create(width, max_height, GL_RED, bitmap.data());
    auto label = "atlas";
    if ((__GLEW_EXT_debug_label)) {
      glLabelObjectEXT(GL_TEXTURE, texture->Handle(), 0, label);
    }
    if ((__GLEW_KHR_debug)) {
      glObjectLabel(GL_TEXTURE, texture->Handle(), -1, label);
    }

    return texture;
  }
};

struct Glyph {
  float xywh[4];
  float offset[4];
};

struct Glyphs {
  Glyph glyphs[128];
};

struct Global {
  float projection[16] = {
      1, 0, 0, 0, //
      0, 1, 0, 0, //
      0, 0, 1, 0, //
      0, 0, 0, 1  //

  };
  float screenSize[2];
  float cellSize[2];
  float atlasSize[2];
  float ascent;
  float descent;

  void UpdateProjection(int width, int height, int cell_width, int cell_height
                        // CursorPosition scroll_top_left: ,
                        // CursorPosition cursor_position: ,
  ) {
    auto cols = width / cell_width;
    auto rows = height / cell_height;

    auto m = projection;
    m[0] = 2.0 / width;
    m[5] = -(2.0 / height);
    m[12] = -1 - cell_width / width * 2;
    m[13] = 1 + cell_height / height * 2;
  }
};

template <typename T> struct TypedUBO {
  std::shared_ptr<glo::UBO> ubo;
  T buffer;

  void Initialize() { ubo = glo::UBO::Create(); }
  void Upload() { ubo->Upload(buffer); }
  uint32_t Handle() { return ubo->Handle(); }
};

class TextImpl {
  std::shared_ptr<glo::VAO> vao_;
  TypedUBO<Global> ubo_global_;
  TypedUBO<Glyphs> ubo_glyphs_;
  std::shared_ptr<glo::ShaderProgram> shader_;
  std::shared_ptr<glo::Texture> font_;

public:
  FontAtlas atlas_;
  bool Load() {
    // shader
    auto vs = glo::ShaderCompile::VertexShader();
    if (!vs->Compile(vs_src, false)) {
      return false;
    }
    auto gs = glo::ShaderCompile::GeometryShader();
    if (!gs->Compile(gs_src, false)) {
      return false;
    }
    auto fs = glo::ShaderCompile::FragmentShader();
    if (!fs->Compile(fs_src, false)) {
      return false;
    }
    shader_ = glo::ShaderProgram::Create();
    if (!shader_->Link(
            {.vs = vs->shader_, .fs = fs->shader_, .gs = gs->shader_})) {
      return false;
    }

    ubo_global_.Initialize();
    ubo_glyphs_.Initialize();

    // vertex buffer
    auto vbo = glo::VBO::Create();
    glo::VertexLayout layouts[] = {
        {{"i_Pos", 0}, 3, 24, 0},
        {{"i_Color", 1}, 3, 24, 12},
    };
    vao_ = glo::VAO::Create(vbo, layouts);

    return true;
  }

  void LoadFont(std::string_view path, float font_size, uint32_t atlas_size) {
    font_ = atlas_.LoadFont(path, font_size, atlas_size);
    for (int i = 0; i < GLYPH_COUNT; ++i) {
      auto &g = atlas_.glyphs[i];
      ubo_glyphs_.buffer.glyphs[i] = {
          .xywh = {(float)g.x0, (float)g.y0, (float)g.x1, (float)g.y1},
          .offset = {(float)g.xoff, (float)g.yoff, (float)g.xoff2,
                     (float)g.yoff2},
      };
    }
    ubo_glyphs_.Upload();
    ubo_global_.buffer.atlasSize[0] = (float)atlas_size;
    ubo_global_.buffer.atlasSize[1] = (float)atlas_size;
    ubo_global_.buffer.ascent = atlas_.ascents[0];
    ubo_global_.buffer.descent = atlas_.descents[0];
  }

  void Commit(std::span<CellVertex> cells) {
    vao_->GetVBO()->DataFromSpan(cells, true);
  }

  void Render(int width, int height, std::chrono::nanoseconds duration,
              int cell_width, int cell_height, int draw_count) {
    if (!font_) {
      return;
    }

    {
      // ubo_global
      ubo_global_.buffer.cellSize[0] = (float)cell_width;
      ubo_global_.buffer.cellSize[1] = (float)cell_height;
      ubo_global_.buffer.screenSize[0] = (float)width;
      ubo_global_.buffer.screenSize[1] = (float)height;
      ubo_global_.buffer.UpdateProjection(width, height, cell_width,
                                          cell_height);
      ubo_global_.Upload();
    }

    {
      auto shader_scope = ScopedBind(shader_);
      auto texture_scope = ScopedBind(font_);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      {
        shader_->SetUBO(0, ubo_global_.Handle());
        shader_->SetUBO(1, ubo_glyphs_.Handle());
        vao_->Draw(GL_POINTS, 0, draw_count);
      }
    }
  }
};

//
// Text
//
CellGrid::CellGrid() : impl_(new TextImpl) {}

CellGrid::~CellGrid() { delete (impl_); }

std::shared_ptr<CellGrid> CellGrid::Create() {
  return std::shared_ptr<CellGrid>(new CellGrid);
}

bool CellGrid::Load(std::string_view path, int font_size, uint32_t atlas_size) {
  if (!impl_->Load()) {
    return false;
  }

  cell_width_ = static_cast<int>(font_size / 2);
  cell_height_ = static_cast<int>(font_size);
  impl_->LoadFont(path, font_size, atlas_size);
  return true;
}

void CellGrid::Clear() {
  cellMap_.clear();
  cells_.clear();
}

void CellGrid::SetCell(Cell cell, std::span<uint32_t> codepoints) {
  auto glyph_index = impl_->atlas_.GlyphIndexFromCodePoint(codepoints);
  auto found = cellMap_.find(cell);
  size_t index;
  if (found != cellMap_.end()) {
    index = found->second;
  } else {
    index = cells_.size();
    cells_.push_back({
        .col = (float)cell.col,
        .row = (float)cell.row,
    });
    cellMap_.insert(std::make_pair(cell, index));
  }

  auto &v = cells_[index];
  v.glyph_index = (float)glyph_index;
  v.color[0] = 1;
  v.color[1] = 1;
  v.color[2] = 1;
}

void CellGrid::Commit() { impl_->Commit(cells_); }

void CellGrid::Render(int width, int height,
                      std::chrono::nanoseconds duration) {
  impl_->Render(width, height, duration, cell_width_, cell_height_,
                cells_.size());
}
