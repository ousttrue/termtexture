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

  ImGui::Begin(name_.c_str()); // Create a window called "Hello,
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

//
// FboWindow
//
FboWindow::FboWindow(std::string_view name, const RenderFunc &render)
    : name_(name), fbo_(new glo::FboRenderer), render_(render) {}

FboWindow::~FboWindow() {}

std::shared_ptr<FboWindow> FboWindow::Create(std::string_view name,
                                             const RenderFunc &render) {
  return std::shared_ptr<FboWindow>(new FboWindow(name, render));
}

void FboWindow::render_fbo(float x, float y, float w, float h,
                           std::chrono::nanoseconds time) {
  assert(w);
  assert(h);
  auto texture =
      fbo_->Begin(static_cast<int>(w), static_cast<int>(h), clear_color_);
  if (texture) {
    ImGui::ImageButton(reinterpret_cast<ImTextureID>(texture), {w, h}, {0, 1},
                       {1, 0}, 0, bg_, tint_);
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

    if (render_) {
      render_(w, h, time);
    }

    fbo_->End();
  }
}

void FboWindow::show(bool *p_open) {
  if (p_open && !*p_open) {
    return;
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
  if (ImGui::Begin(name_.c_str(), p_open,
                   ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoScrollWithMouse)) {
    auto [x, y] = ImGui::GetWindowPos();
    y += ImGui::GetFrameHeight();
    auto [w, h] = ImGui::GetContentRegionAvail();
    render_fbo(x, y, w, h, time_);
  }
  ImGui::End();
  ImGui::PopStyleVar();
}
