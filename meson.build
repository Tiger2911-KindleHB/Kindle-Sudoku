project(
  'kindlesudoku',
  'cpp',
  version: '0.1.0',
  default_options: [
    'cpp_std=c++17',
    'buildtype=release',
    'warning_level=2'
  ]
)

gtk2 = dependency('gtk+-2.0', required: true)
inc = include_directories('src')

cpp = meson.get_compiler('cpp')
static_runtime_link_args = []
foreach arg : ['-static-libstdc++', '-static-libgcc']
  if cpp.has_link_argument(arg)
    static_runtime_link_args += arg
  endif
endforeach

sources = files(
  'src/main.cpp',
  'src/sudoku_engine.cpp',
  'src/game_state.cpp',
  'src/ui.cpp'
)

executable(
  'kindlesudoku',
  sources,
  include_directories: inc,
  dependencies: [gtk2],
  link_args: static_runtime_link_args,
  install: true
)

engine_smoke = executable(
  'engine_smoke',
  files('tests/engine_smoke.cpp', 'src/sudoku_engine.cpp'),
  include_directories: inc,
  link_args: static_runtime_link_args,
  install: false
)

test('engine-smoke', engine_smoke)
