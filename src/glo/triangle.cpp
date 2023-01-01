#include "triangle.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "scoped_binder.h"
#include "shader.h"
#include "ubo.h"
#include "vbo.h"
#include <GL/glew.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <numbers>
#include <optional>
#include <plog/Log.h>
#include <stdint.h>

static const struct {
  float x, y;
  float r, g, b;
} vertices[3] = {{-0.6f, -0.4f, 1.f, 0.f, 0.f},
                 {0.6f, -0.4f, 0.f, 1.f, 0.f},
                 {0.f, 0.6f, 0.f, 0.f, 1.f}};

static const char *vertex_shader_text = R"(#version 450
// uniform mat4 MVP;
layout (location = 0) in vec3 vCol;
layout (location = 1) in vec2 vPos;
layout (location = 0) out vec3 color;
layout (binding = 0) uniform matrix {
    mat4 MVP;
} Mat;
void main()
{
    gl_Position = Mat.MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
}
)";

static const char *fragment_shader_text = R"(#version 450
layout (location = 0) in vec3 color;
layout (location = 0) out vec4 uFragColor;
void main()
{
    uFragColor = vec4(color, 1.0);
}
)";

namespace glo {

class TriangleImpl {
  std::shared_ptr<VAO> vao_;
  std::shared_ptr<UBO> ubo_;
  std::shared_ptr<ShaderProgram> shader_;

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
    auto vs = ShaderCompile::VertexShader();
    if (!vs->Compile(vertex_shader_text)) {
      return false;
    }
    auto fs = ShaderCompile::FragmentShader();
    if (!fs->Compile(fragment_shader_text)) {
      return false;
    }
    shader_ = ShaderProgram::Create();
    if (!shader_->Link(vs->shader_, fs->shader_)) {
      return false;
    }

    ubo_ = UBO::Create();

    // vertex buffer
    auto vbo = VBO::Create(vertices, sizeof(vertices));
    vao_ = VAO::Create(vbo);
    {
      auto vao_bind = ScopedBind(vao_);
      auto vbo_bind = ScopedBind(vbo);
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
    }

    return true;
  }

  void Render(int width, int height, std::chrono::nanoseconds duration) {
    {
      auto ubo_bind = ScopedBind(ubo_);

      const float ANGLE_VELOCITY = std::numbers::pi * 0.000000001f / 5;
      auto rotate_z =
          glm::rotate(duration.count() * ANGLE_VELOCITY, glm::vec3(0, 0, 1));
      auto ratio = (float)width / (float)height;
      auto ortho = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      auto mvp = ortho * rotate_z;

      ubo_->Upload(&mvp, sizeof(mvp));
    }

    {
      auto shader_bind = ScopedBind(shader_);
      auto vao_bind = ScopedBind(vao_);
      ubo_->BindBase(0);
      glDrawArrays(GL_TRIANGLES, 0, 3);
    }
  }
};

Triangle::Triangle() : impl_(new TriangleImpl) {}
Triangle::~Triangle() { delete impl_; }
bool Triangle::Load() { return impl_->Load(); }
void Triangle::Render(int width, int height,
                      std::chrono::nanoseconds duration) {
  impl_->Render(width, height, duration);
}

} // namespace glo