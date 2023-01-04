#include "glfw_window.h"
#include "vterm_keycodes.h"
#include <GLFW/glfw3.h>
#include <glo.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>
#include <stdexcept>
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

static VTermModifier to_vterm_mod(int glfw_mod) {
  auto value = 0; //(int)VTermModifier::VTERM_MOD_NONE;
  if (glfw_mod & GLFW_MOD_SHIFT) {
    value |= VTermModifier::VTERM_MOD_SHIFT;
  }
  if (glfw_mod & GLFW_MOD_CONTROL) {
    value |= VTermModifier::VTERM_MOD_CTRL;
  }
  if (glfw_mod & GLFW_MOD_ALT) {
    value |= VTermModifier::VTERM_MOD_ALT;
  }
  return (VTermModifier)value;
}

static std::optional<VTermKey> to_vterm_key(int glfw_key) {
  switch (glfw_key) {
  case GLFW_KEY_ESCAPE:
    return VTERM_KEY_ESCAPE;
  case GLFW_KEY_ENTER:
    return VTERM_KEY_ENTER;
  case GLFW_KEY_TAB:
    return VTERM_KEY_TAB;
  case GLFW_KEY_BACKSPACE:
    return VTERM_KEY_BACKSPACE;
  case GLFW_KEY_INSERT:
    return VTERM_KEY_INS;
  case GLFW_KEY_DELETE:
    return VTERM_KEY_DEL;
  case GLFW_KEY_RIGHT:
    return VTERM_KEY_RIGHT;
  case GLFW_KEY_LEFT:
    return VTERM_KEY_LEFT;
  case GLFW_KEY_DOWN:
    return VTERM_KEY_DOWN;
  case GLFW_KEY_UP:
    return VTERM_KEY_UP;
  case GLFW_KEY_PAGE_UP:
    return VTERM_KEY_PAGEUP;
  case GLFW_KEY_PAGE_DOWN:
    return VTERM_KEY_PAGEDOWN;
  case GLFW_KEY_HOME:
    return VTERM_KEY_HOME;
  case GLFW_KEY_END:
    return VTERM_KEY_END;
  case GLFW_KEY_F1:
    return (VTermKey)VTERM_KEY_FUNCTION(1);
  case GLFW_KEY_F2:
    return (VTermKey)VTERM_KEY_FUNCTION(2);
  case GLFW_KEY_F3:
    return (VTermKey)VTERM_KEY_FUNCTION(3);
  case GLFW_KEY_F4:
    return (VTermKey)VTERM_KEY_FUNCTION(4);
  case GLFW_KEY_F5:
    return (VTermKey)VTERM_KEY_FUNCTION(5);
  case GLFW_KEY_F6:
    return (VTermKey)VTERM_KEY_FUNCTION(6);
  case GLFW_KEY_F7:
    return (VTermKey)VTERM_KEY_FUNCTION(7);
  case GLFW_KEY_F8:
    return (VTermKey)VTERM_KEY_FUNCTION(8);
  case GLFW_KEY_F9:
    return (VTermKey)VTERM_KEY_FUNCTION(9);
  case GLFW_KEY_F10:
    return (VTermKey)VTERM_KEY_FUNCTION(10);
  case GLFW_KEY_F11:
    return (VTermKey)VTERM_KEY_FUNCTION(11);
  case GLFW_KEY_F12:
    return (VTermKey)VTERM_KEY_FUNCTION(12);
  case GLFW_KEY_F13:
    return (VTermKey)VTERM_KEY_FUNCTION(13);
  case GLFW_KEY_F14:
    return (VTermKey)VTERM_KEY_FUNCTION(14);
  case GLFW_KEY_F15:
    return (VTermKey)VTERM_KEY_FUNCTION(15);
  case GLFW_KEY_F16:
    return (VTermKey)VTERM_KEY_FUNCTION(16);
  case GLFW_KEY_F17:
    return (VTermKey)VTERM_KEY_FUNCTION(17);
  case GLFW_KEY_F18:
    return (VTermKey)VTERM_KEY_FUNCTION(18);
  case GLFW_KEY_F19:
    return (VTermKey)VTERM_KEY_FUNCTION(19);
  case GLFW_KEY_F20:
    return (VTermKey)VTERM_KEY_FUNCTION(20);
  case GLFW_KEY_F21:
    return (VTermKey)VTERM_KEY_FUNCTION(21);
  case GLFW_KEY_F22:
    return (VTermKey)VTERM_KEY_FUNCTION(22);
  case GLFW_KEY_F23:
    return (VTermKey)VTERM_KEY_FUNCTION(23);
  case GLFW_KEY_F24:
    return (VTermKey)VTERM_KEY_FUNCTION(24);
  case GLFW_KEY_F25:
    return (VTermKey)VTERM_KEY_FUNCTION(25);
  default:
    return {};
  }
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {

  auto term = (termtexture::TermTexture *)glfwGetWindowUserPointer(window);
  auto vterm_mod = to_vterm_mod(mods);
  if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_KP_0:
    case GLFW_KEY_KP_1:
    case GLFW_KEY_KP_2:
    case GLFW_KEY_KP_3:
    case GLFW_KEY_KP_4:
    case GLFW_KEY_KP_5:
    case GLFW_KEY_KP_6:
    case GLFW_KEY_KP_7:
    case GLFW_KEY_KP_8:
    case GLFW_KEY_KP_9:
    case GLFW_KEY_KP_DECIMAL:
    case GLFW_KEY_KP_DIVIDE:
    case GLFW_KEY_KP_MULTIPLY:
    case GLFW_KEY_KP_SUBTRACT:
    case GLFW_KEY_KP_ADD:
    case GLFW_KEY_KP_ENTER:
    case GLFW_KEY_KP_EQUAL:
    case GLFW_KEY_LEFT_SHIFT:
    case GLFW_KEY_LEFT_CONTROL:
    case GLFW_KEY_LEFT_ALT:
    case GLFW_KEY_LEFT_SUPER:
    case GLFW_KEY_RIGHT_SHIFT:
    case GLFW_KEY_RIGHT_CONTROL:
    case GLFW_KEY_RIGHT_ALT:
    case GLFW_KEY_RIGHT_SUPER:
    case GLFW_KEY_MENU:
      return;
    }

    if (auto vterm_key = to_vterm_key(key)) {
      term->KeyboardKey(vterm_key.value(), vterm_mod);
    } else {
      if (vterm_mod & VTERM_MOD_CTRL) {
        if (key >= 'A' && key <= 'Z') {
          // lower case
          key += 32;
        }
      } else {
        if (vterm_mod & VTERM_MOD_SHIFT) {
          switch (key) {
          case ';':
            key = ':';
            break;
          }

        } else {
          if (key >= 'A' && key <= 'Z') {
            // lower case
            key += 32;
          } else {
          }
        }
      }
      term->KeyboardUnichar(key, vterm_mod);
    }
  }
}

// static void character_callback(GLFWwindow *window, unsigned int codepoint) {
//   auto term = (termtexture::TermTexture *)glfwGetWindowUserPointer(window);
//   auto mod = VTermModifier::VTERM_MOD_NONE;
//   if (codepoint < 128) {
//     if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) ||
//         glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL)) {
//       mod = (VTermModifier)(mod | (int)VTermModifier::VTERM_MOD_CTRL);
//     }
//   }
//   term->KeyboardUnichar(codepoint, mod);
// }

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
  glfwSetKeyCallback(window_handle, key_callback);
  // glfwSetCharCallback(window_handle, character_callback);

  glo::InitiazlieGlew();
  auto term = termtexture::TermTexture::Create();
  int cell_width = 15;
  int cell_height = 30;
  if (!term->LoadFont(fontfile, cell_width, cell_height)) {
    PLOG_ERROR << "LoadFont: " << fontfile;
    return 2;
  }
  glfwSetWindowUserPointer(window_handle, term.get());

  auto cmd = "cmd.exe";
  if (!term->Launch(cmd)) {
    PLOG_ERROR << "Launch: " << cmd;
    return 3;
  }

  float clear_color[] = {0, 0, 0, 0};
  while (auto time = window.BeginFrame(clear_color)) {
    if(term->IsClosed())
    {
      break;
    }
    auto [width, height] = window.FrameBufferSize();
    term->Render(width, height, time.value());
    window.EndFrame();
  }

  return 0;
}
