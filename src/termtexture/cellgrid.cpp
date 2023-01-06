#include "cellgrid.h"
#include "fontatlas.h"
#include "vterm.h"
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

auto vs_src = R"(#version 420
in vec3 i_Pos;
in vec4 i_Color;
in vec4 i_BgColor;
out vData { 
  vec4 color; 
  vec4 bgColor;
}
vertex;
void main() {
  gl_Position = vec4(i_Pos, 1);
  vertex.color = i_Color;
  vertex.bgColor = i_BgColor;
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

in vData { 
  vec4 color; 
  vec4 bgColor;
}
vertices[];
out vec2 g_TexCoords;
out vec4 g_Color;

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
  vec2 topLeft = gl_in[0].gl_Position.xy * cellSize; // + vec2(0, global.ascent);
  int glyphIndex = int(gl_in[0].gl_Position.z);
  Glyph glyph = glyphs[glyphIndex];
  float l = glyph.xywh.x;
  float t = glyph.xywh.y;
  float r = glyph.xywh.z;
  float b = glyph.xywh.w;
  bool expand = glyph.offset.w != 0;
  float w = expand ? cellSize.x : r - l;
  float h = expand ? cellSize.y : b - t;
  vec2 glyph_offset = expand ? vec2(0, 0) : vec2(glyph.offset.x, glyph.offset.y + global.ascent);

  vec4 cell_0 = vec4(topLeft + glyph_offset, 0, 1);
  vec4 cell_1 = vec4(topLeft + glyph_offset + vec2(0, h), 0.0, 1);
  vec4 cell_2 = vec4(topLeft + glyph_offset + vec2(w, 0), 0.0, 1);
  vec4 cell_3 = vec4(topLeft + glyph_offset + vec2(w, h), 0.0, 1);

  // 0
  gl_Position = global.projection * cell_0;
  g_TexCoords = pixelToUv(l, t);
  g_Color = vertices[0].color;
  EmitVertex();

  // 1
  gl_Position = global.projection * cell_1;
  g_TexCoords = pixelToUv(l, b);
  g_Color = vertices[0].color;
  EmitVertex();

  // 2
  gl_Position = global.projection * cell_2;
  g_TexCoords = pixelToUv(r, t);
  g_Color = vertices[0].color;
  EmitVertex();

  // 3
  gl_Position = global.projection * cell_3;
  g_TexCoords = pixelToUv(r, b);
  g_Color = vertices[0].color;
  EmitVertex();

  EndPrimitive();
}
)";

auto fs_src = R"(#version 460 core

in vec2 g_TexCoords;
in vec4 g_Color;
layout(location = 0) out vec4 FragColor;
uniform sampler2D uTex;

void main() {
  vec4 texcel = texture(uTex, g_TexCoords);
  FragColor = vec4(g_Color.rgb, texcel.x);
  // FragColor = vec4(TexCoords, 0, 1);
}
)";

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

  void UpdateProjection(int width, int height, int cell_width,
                        int cell_height) {
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
  bool Initialize() {
    shader_ = glo::ShaderProgram::Create({vs_src, fs_src, gs_src, false});
    if (!shader_) {
      return false;
    }

    ubo_global_.Initialize();
    ubo_glyphs_.Initialize();

    // vertex buffer
    auto vbo = glo::VBO::Create();
    glo::VertexLayout layouts[] = {
        {{"i_Pos", 0}, GL_FLOAT, 3, 20, 0},
        {{"i_Color", 1}, GL_UNSIGNED_BYTE, 4, 20, 12},
        {{"i_BgColor", 2}, GL_UNSIGNED_BYTE, 4, 20, 16},
    };
    vao_ = glo::VAO::Create(vbo, layouts);

    return true;
  }

  bool LoadFont(std::string_view path, int font_size, uint32_t atlas_size) {
    FontLoader font;
    if (!font.Load(path, static_cast<float>(font_size))) {
      return false;
    }
    atlas_.info = font.info;

    PLOG_INFO << path << std::endl;

    // make a most likely large enough bitmap, adjust to font type, number of
    // sizes and glyphs and oversampling
    auto atlas_width = atlas_size;
    auto atlas_height = atlas_size;
    std::vector<uint8_t> atlas_bitmap(atlas_width * atlas_height);
    assert(atlas_bitmap.size());

    GlyphPackRange ranges[] = {
        {
            .codepoint = 0x20, // space
            .length = 1,
        },
        {
            .codepoint = 0x25A0, // black square
            .length = 1,
        },
        {
            .codepoint = 33,
            .length = 95,
        },
    };
    atlas_.Pack(atlas_bitmap.data(), atlas_width, atlas_height, &font, ranges);
    atlas_.glyphs[0].offset.expand = 1;

    font_ = glo::Texture::Create(atlas_width, atlas_height, GL_RED,
                                 atlas_bitmap.data());
    auto label = "atlas";
    if ((__GLEW_EXT_debug_label)) {
      glLabelObjectEXT(GL_TEXTURE, font_->Handle(), 0, label);
    }
    if ((__GLEW_KHR_debug)) {
      glObjectLabel(GL_TEXTURE, font_->Handle(), -1, label);
    }

    // ubo_glyph
    for (int i = 0; i < atlas_.glyphs.size(); ++i) {
      auto &g = atlas_.glyphs[i];
      ubo_glyphs_.buffer.glyphs[i] = g;
    }
    ubo_glyphs_.Upload();

    // ubo_global
    ubo_global_.buffer.atlasSize[0] = (float)atlas_width;
    ubo_global_.buffer.atlasSize[1] = (float)atlas_height;
    ubo_global_.buffer.ascent = atlas_.info.ascents;
    ubo_global_.buffer.descent = atlas_.info.descents;

    return true;
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
  if (!impl_->Initialize()) {
    return false;
  }

  cell_width_ = static_cast<int>(font_size / 2);
  cell_height_ = static_cast<int>(font_size);
  return impl_->LoadFont(path, font_size, atlas_size);
}

void CellGrid::Clear() {
  cellMap_.clear();
  cells_.clear();
}

void CellGrid::SetCell(CellPos pos, const VTermScreenCell &cell) {
  size_t i = 0;
  for (; i < VTERM_MAX_CHARS_PER_CELL && cell.chars[i]; ++i) {
  }
  auto glyph_index = impl_->atlas_.GlyphIndexFromCodePoint({cell.chars, i});
  auto found = cellMap_.find(pos);
  size_t index;
  if (found != cellMap_.end()) {
    index = found->second;
  } else {
    index = cells_.size();
    cells_.push_back({
        .col = (float)pos.col,
        .row = (float)pos.row,
    });
    cellMap_.insert(std::make_pair(pos, index));
  }

  auto &v = cells_[index];
  v.glyph_index = (float)glyph_index;
  v.fg_color[0] = cell.fg.rgb.red;
  v.fg_color[1] = cell.fg.rgb.green;
  v.fg_color[2] = cell.fg.rgb.blue;
  v.fg_color[3] = 255;
  v.bg_color[0] = cell.bg.rgb.red;
  v.bg_color[1] = cell.bg.rgb.green;
  v.bg_color[2] = cell.bg.rgb.blue;
  v.bg_color[3] = 255;
}

void CellGrid::Commit() { impl_->Commit(cells_); }

void CellGrid::Render(int width, int height,
                      std::chrono::nanoseconds duration) {
  impl_->Render(width, height, duration, cell_width_, cell_height_,
                cells_.size());
}
