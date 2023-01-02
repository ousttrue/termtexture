gl_dep = dependency('gl')
glfw3_dep = dependency('glfw3', default_options: ['install=true'])
imgui_dep = dependency('imgui', default_options: ['default_library=static'])
vterm_dep = dependency('vterm')

executable('glfwterm', [
    'main.cpp',
    'glfw_window.cpp',
    'gui.cpp',
    'gui_widgets.cpp',
    #
    'common_pty_windows.cpp',
    'vterm_object.cpp',
],
    install: true,
    dependencies: [glfw3_dep, imgui_dep, gl_dep, plog_dep, glo_dep, vterm_dep],
)
