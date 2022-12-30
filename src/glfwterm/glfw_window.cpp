#include "glfw_window.h"
#include "plog/Log.h"
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <plog/Logger.h>
#include <stdexcept>

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

Window::Window() {
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    throw std::runtime_error("glfwInit");
  }
}

Window::~Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

GLFWwindow *Window::CreaeWindow(int width, int height, const char *title) {
  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  glsl_version_ = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  glsl_version_ = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
  // GL 3.0 + GLSL 130
  glsl_version_ = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
  // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

  // Create window with graphics context
  window_ = glfwCreateWindow(width, height, title, NULL, NULL);
  if (window_ == NULL) {
    return nullptr;
  }

  glfwMakeContextCurrent(window_);

  PLOG_INFO << "GL_VERSION: " << glGetString(GL_VERSION);
  PLOG_INFO << "GL_VENDOR: " << glGetString(GL_VENDOR);
  PLOG_INFO << "GL_RENDERER: " << glGetString(GL_RENDERER);

  glfwSwapInterval(1); // Enable vsync

  return window_;
}

bool Window::BeginFrame(const float clear_color[4]) {
  if (glfwWindowShouldClose(window_)) {
    return false;
  }
  // Poll and handle events (inputs, window resize, etc.)
  // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
  // tell if dear imgui wants to use your inputs.
  // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
  // your main application, or clear/overwrite your copy of the mouse data.
  // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
  // data to your main application, or clear/overwrite your copy of the
  // keyboard data. Generally you may always pass all inputs to dear imgui,
  // and hide them from your application based on those two flags.
  glfwPollEvents();

  int display_w;
  int display_h;
  glfwGetFramebufferSize(window_, &display_w, &display_h);

  // render
  glViewport(0, 0, display_w, display_h);
  glClearColor(clear_color[0] * clear_color[3], clear_color[1] * clear_color[3],
               clear_color[2] * clear_color[3], clear_color[3]);
  glClear(GL_COLOR_BUFFER_BIT);

  return true;
}

void Window::EndFrame() { glfwSwapBuffers(window_); }
