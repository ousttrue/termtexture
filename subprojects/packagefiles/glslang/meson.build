project('glslang', 'c')

cc = meson.get_compiler('c')
lib_dir = meson.current_source_dir() + '/../lib'
libs = []
libs += cc.find_library('GenericCodeGen', dirs: lib_dir)
libs += cc.find_library('glslang-default-resource-limits', dirs: lib_dir)
libs += cc.find_library('glslang', dirs: lib_dir)
libs += cc.find_library('HLSL', dirs: lib_dir)
libs += cc.find_library('MachineIndependent', dirs: lib_dir)
libs += cc.find_library('OGLCompiler', dirs: lib_dir)
libs += cc.find_library('OSDependent', dirs: lib_dir)
libs += cc.find_library('SPIRV-Tools-opt', dirs: lib_dir)
libs += cc.find_library('SPIRV-Tools', dirs: lib_dir)
libs += cc.find_library('SPIRV', dirs: lib_dir)
libs += cc.find_library('SPVRemapper', dirs: lib_dir)

glslang_dep = declare_dependency(
  dependencies : libs,
  include_directories : include_directories('../include'))
