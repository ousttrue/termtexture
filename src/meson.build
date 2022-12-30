gl_dep = dependency('gl')
glfw3_dep = dependency('glfw3', default_options: ['install=true'])
imgui_dep = dependency('imgui', default_options: ['default_library=static'])
glew_dep = dependency('glew')

executable('glfwterm', [
    'main.cpp',
    'glfw_window.cpp',
    'gui.cpp',
    'gui_widgets.cpp',
    'scene.cpp',
],
install: true,
dependencies: [glfw3_dep, imgui_dep, gl_dep, glew_dep]
)
