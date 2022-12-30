#include "glfw_window.h"
#include "gui.h"

int main(int, char **) {

  Window window;
  auto window_handle =
      window.CreaeWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example");
  if (!window_handle) {
    return 1;
  }

  Gui gui(window_handle, window.glsl_version());

  while (window.BeginFrame(gui.clear_color)) {
    gui.UpdateRender();
    window.EndFrame();
  }

  return 0;
}
