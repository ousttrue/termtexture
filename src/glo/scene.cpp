#include "scene.h"
#include "plog/Log.h"
#include <GL/glew.h>
#include <memory>
#include <plog/Logger.h>

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

namespace scene {

class TriangleImpl {
  GLuint vertex_buffer = 0;
  GLuint vertex_shader = 0;
  GLuint fragment_shader = 0;
  GLuint program = 0;
  GLint mvp_location = -1;
  GLint vpos_location = -1;
  GLint vcol_location = -1;

public:
  TriangleImpl(const TriangleImpl &) = delete;
  TriangleImpl &operator=(const TriangleImpl &) = delete;
  TriangleImpl() {
    glewInit();
    PLOG_INFO << "GLEW_VERSION: " << glewGetString(GLEW_VERSION);

    glGenBuffers(1, &vertex_buffer);
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();
  }
  ~TriangleImpl() {}

  bool Load() {
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    return true;
  }

  void Render() {
    glUseProgram(program);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void *)0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void *)(sizeof(float) * 2));

    float mvp[16] = {
        1, 0, 0, 0, //
        0, 1, 0, 0, //
        0, 0, 1, 0, //
        0, 0, 0, 1, //
    };
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat *)mvp);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
};

Triangle::Triangle() : impl_(new TriangleImpl) {}
Triangle::~Triangle() { delete impl_; }
bool Triangle::Load() { return impl_->Load(); }
void Triangle::Render() { impl_->Render(); }

} // namespace scene
