#include "glo/scene/text.h"
#include "glo/shader.h"
#include "glo/ubo.h"
#include "glo/vao.h"
#include <chrono>
#include <memory>

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

namespace glo {

class TextImpl {
  std::shared_ptr<VAO> vao_;
  std::shared_ptr<UBO> ubo_global_;
  std::shared_ptr<UBO> ubo_glyph_;
  std::shared_ptr<ShaderProgram> shader_;

public:
  bool Load() {
    // shader
    auto vs = ShaderCompile::VertexShader();
    if (!vs->Compile(vs_src)) {
      return false;
    }
    auto gs = ShaderCompile::GeometryShader();
    if (!gs->Compile(vs_src)) {
      return false;
    }
    auto fs = ShaderCompile::FragmentShader();
    if (!fs->Compile(fs_src)) {
      return false;
    }
    shader_ = ShaderProgram::Create();
    if (!shader_->Link(
            {.vs = vs->shader_, .fs = fs->shader_, .gs = gs->shader_})) {
      return false;
    }

    ubo_global_ = UBO::Create();
    ubo_glyph_ = UBO::Create();

    // vertex buffer
    // auto vbo = VBO::Create(vertices, sizeof(vertices));
    // vao_ = VAO::Create(vbo);
    // {
    //   auto vao_bind = ScopedBind(vao_);
    //   auto vbo_bind = ScopedBind(vbo);
    //   if (auto vpos_location = shader_->AttributeLocation("vPos")) {
    //     glEnableVertexAttribArray(vpos_location.value());
    //     glVertexAttribPointer(vpos_location.value(), 2, GL_FLOAT, GL_FALSE,
    //                           sizeof(vertices[0]), (void *)0);
    //   }
    //   if (auto vcol_location = shader_->AttributeLocation("vCol")) {
    //     glEnableVertexAttribArray(vcol_location.value());
    //     glVertexAttribPointer(vcol_location.value(), 3, GL_FLOAT, GL_FALSE,
    //                           sizeof(vertices[0]), (void *)(sizeof(float) *
    //                           2));
    //   }
    // }

    return true;
  }

  void Render(int width, int height, std::chrono::nanoseconds duration) {}
};

Text::Text() : impl_(new TextImpl) {}
Text::~Text() { delete (impl_); }
std::shared_ptr<Text> Text::Create() { return std::shared_ptr<Text>(new Text); }
bool Text::Load() { return impl_->Load(); }
void Text::Render(int width, int height, std::chrono::nanoseconds duration) {
  impl_->Render(width, height, duration);
}

} // namespace glo
