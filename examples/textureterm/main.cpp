#include "glfw_window.h"
#include <glo.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>
#include <termtexture.h>

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

int main(int argc, char **argv) {
  std::string fontfile = "C:/Windows/Fonts/consola.ttf";
  if (argc > 1) {
    fontfile = argv[1];
  }
  static plog::ColorConsoleAppender<plog::MyFormatter> consoleAppender;
  plog::init(plog::verbose, &consoleAppender);
  PLOG_INFO << "start textureterm...";

  Window window;
  auto window_handle = window.CreaeWindow(1280, 720, "textureterm");
  if (!window_handle) {
    return 1;
  }

  glo::InitiazlieGlew();
  auto term = termtexture::TermTexture::Create();
  int cell_width = 15;
  int cell_height = 30;
  if (!term->LoadFont(fontfile, cell_width, cell_height)) {
    PLOG_ERROR << "LoadFont: " << fontfile;
    return 2;
  }

  auto cmd = "cmd.exe";
  if (!term->Launch(cmd)) {
    PLOG_ERROR << "Launch: " << cmd;
    return 3;
  }

  // auto fbo_render = [term](int width, int height,
  //                          std::chrono::nanoseconds time) {
  //   // keyboard input to vterm
  //   {
  //     auto &io = ImGui::GetIO();
  //     for (auto it = io.InputQueueCharacters.begin();
  //          it != io.InputQueueCharacters.end(); ++it) {
  //       term->KeyboardUnichar(*it, VTermModifier::VTERM_MOD_NONE);
  //     }
  //     if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
  //       term->KeyboardKey(VTERM_KEY_ENTER, VTermModifier::VTERM_MOD_NONE);
  //     } else if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
  //       term->KeyboardKey(VTERM_KEY_BACKSPACE,
  //       VTermModifier::VTERM_MOD_NONE);
  //     } else if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
  //       term->KeyboardKey(VTERM_KEY_TAB, VTermModifier::VTERM_MOD_NONE);
  //     }
  //   }
  // };

  float clear_color[] = {0, 0, 0, 0};
  while (auto time = window.BeginFrame(clear_color)) {
    auto [width, height] = window.FrameBufferSize();
    term->Render(width, height, time.value());
    window.EndFrame();
  }

  return 0;
}
