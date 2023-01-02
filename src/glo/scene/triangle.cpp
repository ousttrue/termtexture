#include "glo/scene/triangle.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glo/scoped_binder.h"
#include "glo/shader.h"
#include "glo/ubo.h"
#include "glo/vao.h"
#include <GL/glew.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <numbers>
#include <optional>
#include <plog/Log.h>
#include <stdint.h>

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
    if (!vs->Compile(vertex_shader_text, true)) {
      return false;
    }
    auto fs = ShaderCompile::FragmentShader();
    if (!fs->Compile(fragment_shader_text, true)) {
      return false;
    }
    shader_ = ShaderProgram::Create();
    if (!shader_->Link({.vs = vs->shader_, .fs = fs->shader_})) {
      return false;
    }

    ubo_ = UBO::Create();

    // vertex buffer
    auto vbo = VBO::Create();
    vbo->SetData(sizeof(vertices), vertices);

    // float x, y;
    // float r, g, b;
    // TODO: from shader reflection
    VertexLayout layouts[] = {
        {{"vPos", 0}, 2, 20, 0},
        {{"vCol", 1}, 3, 20, 8},
    };
    vao_ = VAO::Create(vbo, layouts);

    return true;
  }

  using DoubleSeconds = std::chrono::duration<double, std::ratio<1, 1>>;

  void Render(int width, int height, std::chrono::nanoseconds duration) {
    {
      auto ubo_bind = ScopedBind(ubo_);
      DoubleSeconds seconds = duration;

      const float ANGLE_VELOCITY = static_cast<float>(std::numbers::pi) / 5;
      auto rotate_z =
          glm::rotate(static_cast<float>(seconds.count()) * ANGLE_VELOCITY,
                      glm::vec3(0, 0, 1));
      auto ratio = (float)width / (float)height;
      auto ortho = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      auto mvp = ortho * rotate_z;

      ubo_->Upload(mvp);
    }

    {
      auto shader_bind = ScopedBind(shader_);
      ubo_->BindBase(0);
      vao_->Draw(GL_TRIANGLES, 0, 3);
    }
  }
};

Triangle::Triangle() : impl_(new TriangleImpl) {}
Triangle::~Triangle() { delete impl_; }
std::shared_ptr<Triangle> Triangle::Create() {
  return std::shared_ptr<Triangle>(new Triangle);
}
bool Triangle::Load() { return impl_->Load(); }
void Triangle::Render(int width, int height,
                      std::chrono::nanoseconds duration) {
  impl_->Render(width, height, duration);
}

} // namespace glo
