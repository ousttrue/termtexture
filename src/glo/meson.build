glew_dep = dependency('glew')
glm_dep = dependency('glm')
plog_dep = dependency('plog')

glo_inc = include_directories('include')
glo_lib = static_library('glo', [
    'glo.cpp',
    'texture.cpp',
    'fbo.cpp',
    'vbo.cpp',
    'shader.cpp',
    'ubo.cpp',
    'vao.cpp',
    #
    'scene/drawable.cpp',
    'scene/triangle.cpp',
],
include_directories: glo_inc,
dependencies: [glew_dep, plog_dep, spirv_util_dep, glm_dep],
)
glo_dep = declare_dependency(
    link_with: glo_lib,
    include_directories: glo_inc,
    dependencies: [glew_dep, plog_dep, glm_dep]
)
