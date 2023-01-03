glew_dep = dependency('glew')
vterm_dep = dependency('vterm')
stb_dep = dependency('stb')
plog_dep = dependency('plog')

src = files('cellgrid.cpp', 'vterm_object.cpp', 'termtexture.cpp', 'fontatlas.cpp')
if host_machine.system() == 'windows'
    src += files('common_pty_windows.cpp')
else
    src += files('common_pty_posix.cpp')
endif

termtexture_lib = static_library('termtexture', src,
dependencies: [glo_dep, vterm_dep, stb_dep],
)
termtexture_dep = declare_dependency(
    link_with: termtexture_lib,
    include_directories: include_directories('.'),
    dependencies: [vterm_dep],
)
