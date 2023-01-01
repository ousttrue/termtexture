#pragma once
#include <memory>
#include <optional>
#include <stdint.h>

namespace glo {

class ShaderCompile {

  // GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
  int shader_type_;
  ShaderCompile(int shader_type);

public:
  uint32_t shader_ = 0;
  ~ShaderCompile();
  ShaderCompile(const ShaderCompile &) = delete;
  ShaderCompile &operator=(const ShaderCompile &) = delete;
  static std::shared_ptr<ShaderCompile> VertexShader();
  static std::shared_ptr<ShaderCompile> FragmentShader();
  bool Compile(const char *src);
};

class ShaderProgram {
  uint32_t program_ = 0;

  ShaderProgram();

public:
  ~ShaderProgram();
  ShaderProgram(const ShaderProgram &) = delete;
  ShaderProgram &operator=(const ShaderProgram &) = delete;
  static std::shared_ptr<ShaderProgram> Create();
  bool Link(uint32_t vertex_shader, uint32_t fragment_shader);
  void Bind();
  void Unbind();
  std::optional<uint32_t> AttributeLocation(const char *name);
  void SetUniformMatrix(const char *name, const float m[16]);
};

} // namespace glo
