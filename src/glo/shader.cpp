#include "shader.h"
#include <GL/glew.h>
#include <plog/Log.h>

namespace glo {

ShaderCompile::ShaderCompile(int shader_type) {
  shader_ = glCreateShader(shader_type);
}

ShaderCompile::~ShaderCompile() {}
std::shared_ptr<ShaderCompile> ShaderCompile::VertexShader() {
  return std::shared_ptr<ShaderCompile>(new ShaderCompile(GL_VERTEX_SHADER));
}
std::shared_ptr<ShaderCompile> ShaderCompile::FragmentShader() {
  return std::shared_ptr<ShaderCompile>(new ShaderCompile(GL_FRAGMENT_SHADER));
}
bool ShaderCompile::Compile(const char *src) {
  glShaderSource(shader_, 1, &src, nullptr);
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
  ;
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
bool ShaderProgram::Link(uint32_t vertex_shader, uint32_t fragment_shader) {
  glAttachShader(program_, vertex_shader);
  glAttachShader(program_, fragment_shader);
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