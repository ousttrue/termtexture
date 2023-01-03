gl_dep = dependency('gl')
glfw3_dep = dependency('glfw3', default_options: ['install=true'])

glfwwindow_lib = static_library('glfw_window', [
    'glfw_window.cpp',
],
dependencies: [glfw3_dep, gl_dep, plog_dep])
glfwwindow_dep = declare_dependency(
    include_directories: include_directories('.'),
    link_with: glfwwindow_lib,
    dependencies: [glfw3_dep],
)
