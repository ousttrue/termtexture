glslang_dep = dependency('glslang')

spirv_util_lib = shared_library('spirv_util', [
    'spirv_util.cpp',
],
    install: true,
    cpp_args: ['-DSPIRV_UTIL_BUILD=1'],
    override_options: ['buildtype=release'],
    dependencies: [glslang_dep],
)
spirv_util_dep = declare_dependency(
    include_directories: include_directories('.'),
    link_with: spirv_util_lib,
)
