glew_dep = dependency('glew')
glm_dep = dependency('glm')

glo_lib = static_library('glo', [
    'texture.cpp',
    'fbo.cpp',
    'vbo.cpp',
    'shader.cpp',
    'ubo.cpp',
    #
    'triangle.cpp',
],
dependencies: [glew_dep, plog_dep, spirv_util_dep, glm_dep],
)
glo_dep = declare_dependency(
    link_with: glo_lib,
    include_directories: include_directories('.'),
)
