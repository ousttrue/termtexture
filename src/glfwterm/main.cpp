#include "glfw_window.h"
#include "gui.h"
#include <GL/glew.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>

namespace plog {
class MyFormatter {
public:
  static util::nstring header() // This method returns a header for a new file.
                                // In our case it is empty.
  {
    return util::nstring();
  }

  static util::nstring
  format(const Record &record) // This method returns a string from a record.
  {
    tm t;
    util::localtime_s(&t, &record.getTime().time);

    util::nostringstream ss;
    ss << "["
       // hour
       << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour
       << PLOG_NSTR(":")
       // min
       << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min
       << PLOG_NSTR(".")
       // sec
       << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_sec
       << "]"
       // message
       << record.getFunc() << ": " << record.getMessage() << "\n";

    return ss.str();
  }
};
} // namespace plog

int main(int, char **) {
  static plog::ColorConsoleAppender<plog::MyFormatter> consoleAppender;
  plog::init(plog::verbose, &consoleAppender);
  PLOG_INFO << "start GLFWTERM...";

  Window window;
  auto window_handle =
      window.CreaeWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example");
  if (!window_handle) {
    return 1;
  }
  glewInit();

  Gui gui(window_handle, window.glsl_version());

  while (window.BeginFrame(gui.clear_color)) {
    gui.UpdateRender();
    window.EndFrame();
  }

  return 0;
}
