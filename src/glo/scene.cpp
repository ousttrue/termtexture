#include "scene.h"
#include "scoped_binder.h"
#include "shader.h"
#include "vbo.h"
#include <GL/glew.h>
#include <memory>
#include <optional>
#include <plog/Log.h>
#include <stdint.h>

static const struct {
  float x, y;
  float r, g, b;
} vertices[3] = {{-0.6f, -0.4f, 1.f, 0.f, 0.f},
                 {0.6f, -0.4f, 0.f, 1.f, 0.f},
                 {0.f, 0.6f, 0.f, 0.f, 1.f}};

static const char *vertex_shader_text = R"(#version 110
uniform mat4 MVP;
attribute vec3 vCol;
attribute vec2 vPos;
varying vec3 color;
void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
}
)";

static const char *fragment_shader_text = R"(#version 110
varying vec3 color;
void main()
{
    gl_FragColor = vec4(color, 1.0);
}
)";

namespace glo {

class TriangleImpl {
  std::shared_ptr<VBO> vbo_;
  std::shared_ptr<ShaderProgram> shader_;
  std::shared_ptr<ShaderCompile> vs;
  std::shared_ptr<ShaderCompile> fs;

public:
  TriangleImpl(const TriangleImpl &) = delete;
  TriangleImpl &operator=(const TriangleImpl &) = delete;
  TriangleImpl() {
    glewInit();
    PLOG_INFO << "GLEW_VERSION: " << glewGetString(GLEW_VERSION);
  }
  ~TriangleImpl() {}

  bool Load() {
    // shader
    vs = ShaderCompile::VertexShader();
    if (!vs->Compile(vertex_shader_text)) {
      return false;
    }
    fs = ShaderCompile::FragmentShader();
    if (!fs->Compile(fragment_shader_text)) {
      return false;
    }
    shader_ = ShaderProgram::Create();
    if (!shader_->Link(vs->shader_, fs->shader_)) {
      return false;
    }

    // vertex buffer
    vbo_ = VBO::Create(vertices, sizeof(vertices));

    return true;
  }

  void Render() {
    auto vbo_bind = ScopedBind(vbo_);

    if (auto vpos_location = shader_->AttributeLocation("vPos")) {
      glEnableVertexAttribArray(vpos_location.value());
      glVertexAttribPointer(vpos_location.value(), 2, GL_FLOAT, GL_FALSE,
                            sizeof(vertices[0]), (void *)0);
    }
    if (auto vcol_location = shader_->AttributeLocation("vCol")) {
      glEnableVertexAttribArray(vcol_location.value());
      glVertexAttribPointer(vcol_location.value(), 3, GL_FLOAT, GL_FALSE,
                            sizeof(vertices[0]), (void *)(sizeof(float) * 2));
    }

    auto shader_bind = ScopedBind(shader_);
    float mvp[16] = {
        1, 0, 0, 0, //
        0, 1, 0, 0, //
        0, 0, 1, 0, //
        0, 0, 0, 1, //
    };
    shader_->SetUniformMatrix("MVP", mvp);

    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
};

Triangle::Triangle() : impl_(new TriangleImpl) {}
Triangle::~Triangle() { delete impl_; }
bool Triangle::Load() { return impl_->Load(); }
void Triangle::Render() { impl_->Render(); }

} // namespace glo
