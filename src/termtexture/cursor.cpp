#include "cursor.h"
#include <array>
#include <gl/glew.h>
#include <glo/shader.h>
#include <glo/texture.h>
#include <glo/ubo.h>
#include <glo/vao.h>
#include <memory>
#include <span>
#include <stdexcept>

struct Vertex {
  float x, y;
};

auto vs = R"(#version 450
layout (location = 0) in vec2 vPos;
void main()
{
    gl_Position = vec4(vPos, 0.0, 1.0);
}
)";

auto fs = R"(#version 450
layout (location = 0) out vec4 uFragColor;
void main()
{
    uFragColor = vec4(1, 1, 1, 1);
}
)";

struct CursorImpl {
  std::shared_ptr<glo::VBO> vbo_;
  std::shared_ptr<glo::VAO> vao_;
  std::shared_ptr<glo::ShaderProgram> shader_;

public:
  CursorImpl() {
    vbo_ = glo::VBO::Create();
    glo::VertexLayout layouts[] = {
        {{"vPos", 0}, GL_FLOAT, 2, 8, 0},
    };
    vbo_->SetData(8 * 4, nullptr, true);
    vao_ = glo::VAO::Create(vbo_, layouts);

    shader_ = glo::ShaderProgram::Create({vs, fs});
    if (!shader_) {
      throw std::runtime_error("glo::ShaderProgram::Create");
    }
  }

  void Render(const VTermPos &pos, PixelSize screen_size, PixelSize cell_size) {
    // update
    // 0 2
    // 1 3
    float l = -1 + 2 * ((float)pos.col * cell_size.width) / screen_size.width;
    float r =
        -1 + 2 * ((float)(pos.col + 1) * cell_size.width) / screen_size.width;
    float t = 1 - 2 * ((float)pos.row * cell_size.height) / screen_size.height;
    float b =
        1 - 2 * ((float)(pos.row + 1) * cell_size.height) / screen_size.height;
    float vertices[8] = {
        l, t, //
        l, b, //
        r, t, //
        r, b, //
    };
    vbo_->SetSubData(vertices, 0, sizeof(vertices));
    // render
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    vao_->Draw(GL_TRIANGLE_STRIP, 0, 4);
  }
};

Cursor::Cursor() : impl_(new CursorImpl) {}
Cursor::~Cursor() { delete impl_; }
std::shared_ptr<Cursor> Cursor::Create() {
  return std::shared_ptr<Cursor>(new Cursor);
}
void Cursor::Render(const VTermPos &pos, PixelSize screen_size,
                    PixelSize cell_size) {
  impl_->Render(pos, screen_size, cell_size);
}
