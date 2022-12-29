gl_dep = dependency('gl')
glfw3_dep = dependency('glfw3', default_options: ['install=true'])
imgui_dep = dependency('imgui')

executable('glfwterm', [
    'main.cpp'
],
install: true,
dependencies: [glfw3_dep, imgui_dep, gl_dep]
)
