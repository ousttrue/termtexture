#include "glo/scene/drawable.h"
#include "glo/scoped_binder.h"
#include "glo/shader.h"
#include "glo/ubo.h"
#include "glo/vao.h"
#include <GL/glew.h>
#include <chrono>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <numbers>
#include <optional>
#include <plog/Log.h>
#include <stdint.h>

namespace glo {

using DoubleSeconds = std::chrono::duration<double, std::ratio<1, 1>>;

Drawable::Drawable(const std::shared_ptr<ShaderProgram> &shader,
                   const std::shared_ptr<VAO> &vao)
    : shader_(shader), vao_(vao), ubo_(UBO::Create()) {}

Drawable::~Drawable() {}

std::shared_ptr<Drawable> Drawable::Create(const ShaderSources &src,
                                           const void *vertices,
                                           size_t vertices_size,
                                           std::span<VertexLayout> layouts) {

  auto shader = ShaderProgram::Create(src);
  if (!shader) {
    return nullptr;
  }

  auto vbo = VBO::Create();
  vbo->SetData(vertices_size, vertices);

  auto vao = VAO::Create(vbo, layouts);

  return std::shared_ptr<Drawable>(new Drawable(shader, vao));
}

void Drawable::Render(std::chrono::nanoseconds duration,
                      const glm::mat4 &projection) {

  {
    auto ubo_bind = ScopedBind(ubo_);
    DoubleSeconds seconds = duration;

    const float ANGLE_VELOCITY = static_cast<float>(std::numbers::pi) / 5;
    auto rotate_z =
        glm::rotate(static_cast<float>(seconds.count()) * ANGLE_VELOCITY,
                    glm::vec3(0, 0, 1));
    auto mvp = projection * rotate_z;

    ubo_->Upload(mvp);
  }

  {
    auto shader_bind = ScopedBind(shader_);
    ubo_->BindBase(0);
    vao_->Draw(GL_TRIANGLES, 0, 3);
  }
}

} // namespace glo
