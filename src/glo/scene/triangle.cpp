#include "glo/scene/drawable.h"
#include <gl/glew.h>

struct Vertex {
  float x, y;
  float r, g, b;
};

static const Vertex vertices[3] = {{-0.6f, -0.4f, 1.f, 0.f, 0.f},
                                   {0.6f, -0.4f, 0.f, 1.f, 0.f},
                                   {0.f, 0.6f, 0.f, 0.f, 1.f}};

auto vertex_shader_text = R"(#version 450
layout (location = 0) in vec2 vPos;
layout (location = 1) in vec3 vCol;
layout (location = 0) out vec3 color;
// uniform mat4 MVP;
layout (binding = 0) uniform matrix {
    mat4 MVP;
} Mat;
void main()
{
    gl_Position = Mat.MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
}
)";

auto fragment_shader_text = R"(#version 450
layout (location = 0) in vec3 color;
layout (location = 0) out vec4 uFragColor;
void main()
{
    uFragColor = vec4(color, 1.0);
}
)";

namespace glo {

std::shared_ptr<Drawable> CreateTriangle() {
  // float x, y;
  // float r, g, b;
  VertexLayout layouts[] = {
      {{"vPos", 0}, GL_FLOAT, 2, 20, 0},
      {{"vCol", 1}, GL_FLOAT, 3, 20, 8},
  };
  return Drawable::Create(
      {
          .vs = vertex_shader_text,
          .fs = fragment_shader_text,
      },
      vertices, sizeof(vertices), layouts);
}

} // namespace glo