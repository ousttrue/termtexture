gl_dep = dependency('gl')
glfw3_dep = dependency('glfw3', default_options: ['install=true'])
imgui_dep = dependency('imgui', default_options: ['default_library=static'])

executable('glfwterm', [
    'main.cpp',
    'glfw_window.cpp',
    'gui.cpp',
],
install: true,
dependencies: [glfw3_dep, imgui_dep, gl_dep]
)
