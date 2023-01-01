glew_dep = dependency('glew')

glo_lib = static_library('glo', [
    'scene.cpp',
    'texture.cpp',
    'fbo.cpp',
    'vbo.cpp',
    'shader.cpp',
    'ubo.cpp',
],
dependencies: [glew_dep, plog_dep, spirv_util_dep],
)
glo_dep = declare_dependency(
    link_with: glo_lib,
    include_directories: include_directories('.'),
)
