#pragma once
#include <chrono>
#include <glm/glm.hpp>
#include <glo/shader.h>
#include <glo/ubo.h>
#include <glo/vao.h>
#include <memory>

namespace glo {

class Drawable {

  std::shared_ptr<ShaderProgram> shader_;
  std::shared_ptr<VAO> vao_;
  std::shared_ptr<UBO> ubo_;
  Drawable(const std::shared_ptr<ShaderProgram> &shader,
           const std::shared_ptr<VAO> &vao);

public:
  ~Drawable();
  static std::shared_ptr<Drawable> Create(const ShaderSources &src,
                                          const void *vertices,
                                          size_t vertices_size,
                                          std::span<VertexLayout> layouts);
  Drawable(const Drawable &) = delete;
  Drawable &operator=(const Drawable &) = delete;
  void Render(std::chrono::nanoseconds duration, const glm::mat4 &projection);
};

} // namespace glo
