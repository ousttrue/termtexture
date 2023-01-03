imgui_dep = dependency('imgui', default_options: ['default_library=static'])

executable('glfwterm', [
    'main.cpp',
    'gui.cpp',
    'gui_widgets.cpp',
],
    install: true,
    dependencies: [glfwwindow_dep, imgui_dep, plog_dep, glo_dep, termtexture_dep],
)
