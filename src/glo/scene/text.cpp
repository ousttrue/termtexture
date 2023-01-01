#include "glo/scene/text.h"
#include "glo/readallbytes.h"
#include "glo/scoped_binder.h"
#include "glo/shader.h"
#include "glo/texture.h"
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

class FontAtlas {

  // allocator: std.mem.Allocator,
  // glyphs: [GLYPH_COUNT]stb.stbtt_packedchar = undefined,
  // ascents: [GLYPH_COUNT]f32 = undefined,
  // descents: [GLYPH_COUNT]f32 = undefined,
  // linegaps: [GLYPH_COUNT]f32 = undefined,

public:
  // pub fn new(allocator: std.mem.Allocator) *Self {
  //     var self = allocator.create(Self) catch unreachable;
  //     self.* = Self{
  //         .allocator = allocator,
  //     };
  //     return self;
  // }

  // pub fn delete(self: *Self) void {
  //     self.allocator.destroy(self);
  // }

  // pub fn glyphIndexFromCodePoint(self: Self, c: u16) usize {
  //     _ = self;
  //     if (c < 32) {
  //         return 0;
  //     }
  //     return c - 32;
  // }

  // https://gist.github.com/vassvik/f442a4cc6127bc7967c583a12b148ac9
  std::shared_ptr<Texture> LoadFont(std::string_view path, float font_size,
                                    uint32_t atlas_size) {
    auto ttf_buffer = ReadAllBytes<char>(path);
    if (ttf_buffer.empty()) {
      return {};
    }

    return nullptr;

    //     var ranges = [_]stb.stbtt_pack_range{
    //         .{ .font_size = font_size, .first_unicode_codepoint_in_range =
    //         32, .num_chars = self.glyphs.len, .chardata_for_range =
    //         &self.glyphs[0]
    //         },
    //     };

    //     // make a most likely large enough bitmap, adjust to font type,
    //     number of sizes and glyphs and oversampling const width = atlas_size;
    //     const max_height = atlas_size; var bitmap = try
    //     self.allocator.alloc(u8, max_height * width); defer
    //     self.allocator.free(bitmap);

    //     // do the packing, based on the ranges specified
    //     var pc: stb.stbtt_pack_context = undefined;
    //     _ = stb.stbtt_PackBegin(&pc, &bitmap[0], @intCast(c_int, width),
    //     @intCast(c_int, max_height), 0, 1, null);
    //     stb.stbtt_PackSetOversampling(&pc, 1, 1); // say, choose 3x1
    //     oversampling for subpixel positioning _ =
    //     stb.stbtt_PackFontRanges(&pc, &ttf_buffer[0], 0, &ranges[0],
    //     ranges.len); stb.stbtt_PackEnd(&pc);

    //     // get the global metrics for each size/range
    //     var info: stb.stbtt_fontinfo = undefined;
    //     _ = stb.stbtt_InitFont(&info, &ttf_buffer[0],
    //     stb.stbtt_GetFontOffsetForIndex(&ttf_buffer[0], 0));

    //     for (ranges) |*r, i| {
    //         const scale = stb.stbtt_ScaleForPixelHeight(&info, r.font_size);
    //         _ = scale;
    //         var a: c_int = undefined;
    //         var d: c_int = undefined;
    //         var l: c_int = undefined;
    //         stb.stbtt_GetFontVMetrics(&info, &a, &d, &l);
    //         self.ascents[i] = @intToFloat(f32, a) * scale;
    //         self.descents[i] = @intToFloat(f32, d) * scale;
    //         self.linegaps[i] = @intToFloat(f32, l) * scale;
    //     }

    //     // for (ranges) |*r, j| {
    //     //     std.log.debug("size    {}:", .{r.font_size});
    //     //     std.log.debug("ascent  {}:", .{self.ascents[j]});
    //     //     std.log.debug("descent {}:", .{self.descents[j]});
    //     //     std.log.debug("linegap {}:", .{self.linegaps[j]});
    //     //     for (self.glyphs) |g, i| {
    //     //         std.log.debug("    '{}':  (x0,y0) = ({},{}),  (x1,y1) =
    //     ({},{}),  (xoff,yoff) = ({},{}),  (xoff2,yoff2) = ({},{}),  xadvance
    //     =
    //     {}", .{
    //     //             32 + i,
    //     //             g.x0,
    //     //             g.y0,
    //     //             g.x1,
    //     //             g.y1,
    //     //             g.xoff,
    //     //             g.yoff,
    //     //             g.xoff2,
    //     //             g.yoff2,
    //     //             g.xadvance,
    //     //         });
    //     //     }
    //     // }

    //     return glo.Texture.init(width, max_height, 1, bitmap);
  }
};

class TextImpl {
  std::shared_ptr<VAO> vao_;
  std::shared_ptr<UBO> ubo_global_;
  std::shared_ptr<UBO> ubo_glyph_;
  std::shared_ptr<ShaderProgram> shader_;
  FontAtlas atlas_;
  std::shared_ptr<Texture> font_;

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

  void LoadFont(std::string_view path, float font_size, uint32_t atlas_size) {
    font_ = atlas_.LoadFont(path, font_size, atlas_size);
    // for (self.atlas.glyphs)
    //   | *g, i | {
    //     self.ubo_glyphs.buffer.glyphs[i] =.{
    //         .xywh =.{@intToFloat(f32, g.x0), @intToFloat(f32, g.y0),
    //                  @intToFloat(f32, g.x1), @intToFloat(f32, g.y1)},
    //         .offset =.{g.xoff, g.yoff, g.xoff2, g.yoff2},
    //     };
    //   }
    // self.ubo_glyphs.upload();
    // self.ubo_global.buffer.atlasSize =.{
    //   @intToFloat(f32, atlas_size), @intToFloat(f32, atlas_size)
    // };
    // self.ubo_global.buffer.ascent = self.atlas.ascents[0];
    // self.ubo_global.buffer.descent = self.atlas.descents[0];
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
bool Text::Load(std::string_view path, float font_size, uint32_t atlas_size) {
  if (!impl_->Load()) {
    return false;
  }
  impl_->LoadFont(path, font_size, atlas_size);
  return true;
}
void Text::Render(int width, int height, std::chrono::nanoseconds duration) {
  impl_->Render(width, height, duration);
}

} // namespace glo
