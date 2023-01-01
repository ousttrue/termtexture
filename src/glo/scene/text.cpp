#include "glo/scene/text.h"
#include "glo/scoped_binder.h"
#include "glo/shader.h"
#include "glo/ubo.h"
#include "glo/vao.h"
#include <chrono>
#include <gl/glew.h>
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
    if (!vs->Compile(vs_src, false)) {
      return false;
    }
    auto gs = ShaderCompile::GeometryShader();
    if (!gs->Compile(gs_src, false)) {
      return false;
    }
    auto fs = ShaderCompile::FragmentShader();
    if (!fs->Compile(fs_src, false)) {
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
    auto vbo = VBO::Create();
    VertexLayout layouts[] = {
        {{"i_Pos", 0}, 3, 24, 0},
        {{"i_Color", 1}, 3, 24, 12},
    };
    vao_ = VAO::Create(vbo, layouts);

    return true;
  }

  void Render(int width, int height, std::chrono::nanoseconds duration) {

    // ubo_global
    // self.ubo_global.buffer.cellSize = .{ @intToFloat(f32, self.cell_width),
    // @intToFloat(f32, self.cell_height) }; self.ubo_global.buffer.screenSize =
    // .{ @intToFloat(f32, mouse_input.width), @intToFloat(f32,
    // mouse_input.height) }; self.ubo_global.buffer.projection = .{
    //     1, 0, 0, 0,
    //     0, 1, 0, 0,
    //     0, 0, 1, 0,
    //     0, 0, 0, 1,
    // };
    // self.scroll_top_left = screenToDevice(
    //     &self.ubo_global.buffer.projection,
    //     @intCast(i32, mouse_input.width),
    //     @intCast(i32, mouse_input.height),
    //     @intCast(i32, self.cell_width),
    //     @intCast(i32, self.cell_height),
    //     self.scroll_top_left,
    //     self.layout.cursor_position,
    // );
    // self.ubo_global.upload();

    auto shader_scope = ScopedBind(shader_);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // if (self.texture)
    //   | *texture | { texture.bind(); }
    // self.shader.setUbo("Global", 0, self.ubo_global.handle);
    // self.shader.setUbo("Glyphs", 1, self.ubo_glyphs.handle);

    // if (self.document_gen != self.layout_gen) {
    //   self.layout_gen = self.document_gen;
    //   if (self.document)
    //     | document | {
    //       // const draw_count = self.layout.layout(document.utf16Slice(),
    //       // self.atlas);
    //       const draw_count =
    //           self.layout.layoutTokens(document.utf8Slice(), self.atlas);
    //       self.draw_count = draw_count;
    //     }
    //   else {
    //     self.draw_count = 0;
    //   }
    //   self.vbo.update(self.layout.cells, .{});
    // }

    vao_->Draw(GL_POINTS, 0, 1);
  }
};

Text::Text() : impl_(new TextImpl) {}
Text::~Text() { delete (impl_); }
std::shared_ptr<Text> Text::Create() { return std::shared_ptr<Text>(new Text); }
bool Text::Load() { return impl_->Load(); }
void Text::Render(int width, int height, std::chrono::nanoseconds duration) {
  impl_->Render(width, height, duration);
}

} // namespace glo
