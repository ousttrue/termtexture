#include "gui.h"
#include "GLFW/glfw3.h"
#include "common_pty.h"
#include "glfw_window.h"
#include "gui_widgets.h"
#include "plog/Log.h"
#include <chrono>
#include <glm/gtx/transform.hpp>
#include <glo/scene/triangle.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <termtexture.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) &&                                 \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

Gui::Gui(GLFWwindow *window, std::string_view glsl_version,
         const std::string &fontfile) {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
  // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsClassic();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(std::string(glsl_version).c_str());

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please
  // handle those errors in your application (e.g. use an assertion, or
  // display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and
  // stored into a texture when calling
  // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame
  // below will call.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  // ImFont *font =
  //     io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  //                                  NULL,
  //                                  io.Fonts->GetGlyphRangesJapanese());
  // IM_ASSERT(font != NULL);

  // 1. Show the big demo window (Most of the sample code is in
  // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
  // ImGui!).
  windows_.push_back({&ImGui::ShowDemoWindow});

  // 3. Show another simple window.
  windows_.push_back({&another_window});

  // 2. Show a simple window that we create ourselves. We use a Begin/End pair
  // to created a named window.
  windows_.push_back({
      .on_show =
          simple_window{
              .name_ = "Hello, world!",
              .show_demo_window_ = &windows_.front().show,
              .show_another_window_ = &windows_.back().show,
              .clear_color = clear_color,
          },
      .use_show = false,
  });

  {
    auto triangle = glo::CreateTriangle();
    if (!triangle) {
      throw std::runtime_error("Triangle::Load");
    }
    auto fbo_render = [triangle](int width, int height,
                                 std::chrono::nanoseconds time) {

      auto ratio = (float)width / (float)height;
      auto ortho = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
      triangle->Render(time, ortho);
    };

    auto fbo_window = FboWindow::Create("triangle", fbo_render);

    windows_.push_back(GuiWindow{
        .on_show = [fbo_window](bool *p_open) { fbo_window->show(p_open); },
        .use_show = false,
        .on_update =
            [fbo_window](std::chrono::nanoseconds time) {
              fbo_window->update(time);
            },
    });
  }

  {
    auto term = termtexture::TermTexture::Create();
    int cell_width = 15;
    int cell_height = 30;
    if (!term->LoadFont(fontfile, cell_width, cell_height)) {
      throw std::runtime_error("TermTexture::LoadFont");
    }

    auto cmd = "cmd.exe";
    if (!term->Launch(cmd)) {
      PLOG_ERROR << "fail: " << cmd;
    }

    auto fbo_render = [term](int width, int height,
                             std::chrono::nanoseconds time) {
      // keyboard input to vterm
      {
        auto &io = ImGui::GetIO();
        for (auto it = io.InputQueueCharacters.begin();
             it != io.InputQueueCharacters.end(); ++it) {
          term->KeyboardUnichar(*it, VTermModifier::VTERM_MOD_NONE);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
          term->KeyboardKey(VTERM_KEY_ENTER, VTermModifier::VTERM_MOD_NONE);
        } else if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
          term->KeyboardKey(VTERM_KEY_BACKSPACE, VTermModifier::VTERM_MOD_NONE);
        } else if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
          term->KeyboardKey(VTERM_KEY_TAB, VTermModifier::VTERM_MOD_NONE);
        }
      }

      term->Render(width, height, time);
    };

    auto fbo_window = FboWindow::Create("text", fbo_render);
    windows_.push_back({
        .on_show = [fbo_window](bool *p_open) { fbo_window->show(p_open); },
        .use_show = false,
        .on_update =
            [fbo_window](std::chrono::nanoseconds time) {
              fbo_window->update(time);
            },
    });
  }
}

Gui::~Gui() {
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Gui::UpdateRender(std::chrono::nanoseconds time) {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // std::chrono::duration_cast<std::chrono::nanoseconds>(seconds)
  for (auto &window : windows_) {
    window.Update(time);
    window.Show();
  }

  // Rendering
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
