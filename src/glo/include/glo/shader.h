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
  static std::shared_ptr<ShaderCompile> GeometryShader();
  bool Compile(const char *src, bool use_spirv);
};

struct ShaderSources {
  const char *vs = nullptr;
  const char *fs = nullptr;
  const char *gs = nullptr;
  bool use_spirv = true;
};

struct Shaders {
  uint32_t vs = 0;
  uint32_t fs = 0;
  uint32_t gs = 0;
};

class ShaderProgram {
  uint32_t program_ = 0;

  ShaderProgram();

public:
  ~ShaderProgram();
  ShaderProgram(const ShaderProgram &) = delete;
  ShaderProgram &operator=(const ShaderProgram &) = delete;
  static std::shared_ptr<ShaderProgram> Create(const ShaderSources &src);
  bool Link(Shaders shaders);
  void Bind();
  void Unbind();
  std::optional<uint32_t> AttributeLocation(const char *name);
  void SetUniformMatrix(const char *name, const float m[16]);
  void SetUBO(int binding_point, uint32_t ubo);
};

} // namespace glo
