#include <GL/glew.h>
#include <gui_widgets.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <memory>

void another_window(bool *p_open) {
  ImGui::Begin("Another Window",
               p_open); // Pass a pointer to our bool variable
                        // (the window will have a closing
                        // button that will
  // clear the bool when clicked)
  ImGui::Text("Hello from another window!");
  if (ImGui::Button("Close Me"))
    *p_open = false;
  ImGui::End();
}

void simple_window::operator()(bool *) {

  static float f = 0.0f;
  static int counter = 0;

  ImGui::Begin("Hello, world!"); // Create a window called "Hello,
                                 // world!" and append into it.

  ImGui::Text("This is some useful text."); // Display some text (you can
                                            // use a format strings too)
  ImGui::Checkbox(
      "Demo Window",
      show_demo_window_); // Edit bools storing our window open/close
  // state
  ImGui::Checkbox("Another Window", show_another_window_);

  ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
  // Edit 1 float using a slider from 0.0f to 1.0f
  ImGui::ColorEdit3("clear color", clear_color);
  // Edit 3 floats representing a color

  if (ImGui::Button("Button")) // Buttons return true when clicked (most
    // widgets return true when
    //  edited/activated)
    counter++;
  ImGui::SameLine();
  ImGui::Text("counter = %d", counter);

  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::End();
}

void fbo_window::operator()(bool *p_open) {
  if (p_open && !*p_open) {
    return;
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
  if (ImGui::Begin("render target", p_open,
                   ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoScrollWithMouse)) {
    auto [x, y] = ImGui::GetWindowPos();
    y += ImGui::GetFrameHeight();
    auto [w, h] = ImGui::GetContentRegionAvail();
    show_fbo(x, y, w, h);
  }
  ImGui::End();
  ImGui::PopStyleVar();
}

class Texture {
  int width_;
  int height_;
  // GL_RGBA(32bit) or GL_RED(8bit graysclale)
  int pixel_type_;
  GLuint handle_;

public:
  Texture(int width, int height, const uint8_t *data = nullptr,
          int pixel_type = GL_RGBA)
      : width_(width), height_(height), pixel_type_(pixel_type) {
    glGenTextures(1, &handle_);
    // logger.debug(f'Texture: {self.handle}')

    Bind();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // glPixelStorei(GL_UNPACK_ROW_LENGTH, width)
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, pixel_type_, width_, height_, 0, pixel_type_,
                 GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Unbind();
  }

  ~Texture() {
    // logger.debug(f'{self.handle}')
    glDeleteTextures(1, &handle_);
  }
  GLuint Handle() const { return handle_; }
  int Width() const { return width_; }
  int Height() const { return height_; }
  void update(int x, int y, int w, int h, const uint8_t *data) {
    Bind();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width_);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, y);

    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, pixel_type_, GL_UNSIGNED_BYTE,
                    data);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    Unbind();
  }
  void Bind() { glBindTexture(GL_TEXTURE_2D, handle_); }
  void Unbind() { glBindTexture(GL_TEXTURE_2D, 0); }
};

class Fbo {
  Texture texture_;
  GLuint fbo_ = 0;
  GLuint depth_ = 0;

public:
  Texture *Texture() { return &texture_; }
  Fbo(int width, int height, bool use_depth = true) : texture_(width, height) {
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture_.Handle(), 0);
    unsigned int buf = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &buf);

    if (use_depth) {
      glGenRenderbuffers(1, &depth_);
      glBindRenderbuffer(GL_RENDERBUFFER, depth_);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                GL_RENDERBUFFER, depth_);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // LOGGER.debug(f'fbo: {self.fbo}, texture: {self.texture}, depth:
    // {self.depth}')
  }
  ~Fbo() {
    // LOGGER.debug(f'fbo: {self.fbo}')
    glDeleteFramebuffers(1, &fbo_);
  }
  void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, fbo_); }
  void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
};

class FboRenderer {
  std::shared_ptr<Fbo> fbo_;

public:
  FboRenderer() {}
  ~FboRenderer() {}
  GLuint Begin(int width, int height, const float color[4]) {
    if (width == 0 || height == 0) {
      return 0;
    }

    if (fbo_) {
      if (fbo_->Texture()->Width() != width ||
          fbo_->Texture()->Height() != height) {
        fbo_ = nullptr;
      }
    }
    if (!fbo_) {
      fbo_ = std::make_shared<Fbo>(width, height);
    }

    fbo_->Bind();
    glViewport(0, 0, width, height);
    glScissor(0, 0, width, height);
    glClearColor(color[0] * color[3], color[1] * color[3], color[2] * color[3],
                 color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    return fbo_->Texture()->Handle();
  }
  void End() { fbo_->Unbind(); }
};

fbo_window::fbo_window() : fbo_(new FboRenderer) {}

void fbo_window::show_fbo(float x, float y, float w, float h) {
  assert(w);
  assert(h);
  auto texture =
      fbo_->Begin(static_cast<int>(w), static_cast<int>(h), clear_color);
  if (texture) {
    ImGui::ImageButton(reinterpret_cast<ImTextureID>(texture), {w, h}, {0, 1},
                       {1, 0}, 0, bg, tint);
    ImGui::ButtonBehavior(ImGui::GetCurrentContext()->LastItemData.Rect,
                          ImGui::GetCurrentContext()->LastItemData.ID, 0, 0,
                          ImGuiButtonFlags_MouseButtonMiddle |
                              ImGuiButtonFlags_MouseButtonRight);

    // auto &io = ImGui::GetIO();

    // mouse_input = MouseInput(
    //     (int(io.MousePos.x) - x), (int(io.MousePos.y) - y),
    //     w, h,
    //     io.MouseDown[0], io.MouseDown[1], io.MouseDown[2],
    //     ImGui.IsItemActive(), ImGui.IsItemHovered(), int(io.MouseWheel))
    // self.mouse_event.process(mouse_input)

    // if self.render:
    //     self.render(mouse_input)
    // else:
    //     self.mouse_event.debug_draw()

    fbo_->End();
  }
}
