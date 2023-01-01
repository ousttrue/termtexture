#include "glo/shader.h"
#include "spirv_util.h"
#include <GL/glew.h>
#include <plog/Log.h>

namespace glo {

ShaderCompile::ShaderCompile(int shader_type) : shader_type_(shader_type) {
  shader_ = glCreateShader(shader_type);
}

ShaderCompile::~ShaderCompile() {}
std::shared_ptr<ShaderCompile> ShaderCompile::VertexShader() {
  return std::shared_ptr<ShaderCompile>(new ShaderCompile(GL_VERTEX_SHADER));
}
std::shared_ptr<ShaderCompile> ShaderCompile::FragmentShader() {
  return std::shared_ptr<ShaderCompile>(new ShaderCompile(GL_FRAGMENT_SHADER));
}
std::shared_ptr<ShaderCompile> ShaderCompile::GeometryShader() {
  return std::shared_ptr<ShaderCompile>(new ShaderCompile(GL_GEOMETRY_SHADER));
}
bool ShaderCompile::Compile(const char *src, bool use_spirv) {
  if (use_spirv) {
    SPIRV_Initialize();
    SpirvCompiler *compiler = nullptr;
    switch (shader_type_) {
    case GL_VERTEX_SHADER:
      compiler = SPIRV_COMPILER_Create_VS();
      break;

    case GL_FRAGMENT_SHADER:
      compiler = SPIRV_COMPILER_Create_FS();
      break;

    case GL_GEOMETRY_SHADER:
      compiler = SPIRV_COMPILER_Create_GS();
      break;

    default:
      PLOG_ERROR << "unknown shader: " << shader_type_;
      return false;
    }

    unsigned int size = 0;
    auto spirv = SPIRV_COMPILER_Compile(compiler, src, &size);
    if (!size) {
      PLOG_ERROR << SPIRV_COMPILER_GetError(compiler) << "\n" << src;
      SPIRV_COMPILER_Destroy(compiler);
      return false;
    }

    glShaderBinary(1, &shader_, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, spirv,
                   size * sizeof(unsigned int));
    glSpecializeShaderARB(shader_, "main", 0, nullptr, nullptr);
    SPIRV_COMPILER_Destroy(compiler);
    SPIRV_Finalize();
  } else {
    glShaderSource(shader_, 1, &src, nullptr);
  }

  glCompileShader(shader_);
  GLint isCompiled = 0;
  glGetShaderiv(shader_, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader_, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader_, maxLength, &maxLength, &errorLog[0]);
    errorLog.push_back(0);

    PLOG_WARNING << errorLog.data();

    return false;
  }
  return true;
}

ShaderProgram::ShaderProgram() {
  program_ = glCreateProgram();
  PLOG_INFO << "program: " << program_;
}

ShaderProgram::~ShaderProgram() {
  PLOG_INFO << "program: " << program_;
  glDeleteProgram(program_);
}

std::shared_ptr<ShaderProgram> ShaderProgram::Create() {
  return std::shared_ptr<ShaderProgram>(new ShaderProgram);
}
bool ShaderProgram::Link(Shaders shaders) {
  if (shaders.vs)
    glAttachShader(program_, shaders.vs);
  if (shaders.fs)
    glAttachShader(program_, shaders.fs);
  if (shaders.gs)
    glAttachShader(program_, shaders.gs);

  glLinkProgram(program_);
  GLint isLinked = 0;
  glGetProgramiv(program_, GL_LINK_STATUS, &isLinked);
  if (isLinked == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::vector<GLchar> infoLog(maxLength);
    glGetProgramInfoLog(program_, maxLength, &maxLength, &infoLog[0]);

    infoLog.push_back(0);
    PLOG_WARNING << infoLog.data();

    return false;
  }
  return true;
}
void ShaderProgram::Bind() { glUseProgram(program_); }
void ShaderProgram::Unbind() { glUseProgram(0); }

std::optional<uint32_t> ShaderProgram::AttributeLocation(const char *name) {
  auto location = glGetAttribLocation(program_, name);
  if (location < 0) {
    return {};
  }
  return location;
}

void ShaderProgram::SetUniformMatrix(const char *name, const float m[16]) {
  auto location = glGetUniformLocation(program_, name);
  glUniformMatrix4fv(location, 1, GL_FALSE, m);
}

} // namespace glo