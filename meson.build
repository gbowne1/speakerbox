project('speakerbox', 'cpp',
  version: '0.0.1',
  default_options: ['cpp_std=c++17', 'warning_level=3', 'optimization=3'])

add_global_arguments('-D_XOPEN_SOURCE=700', '-D_GNU_SOURCE', '-DPI=3.14159265358979323846', language: 'cpp')

inc = include_directories('include')

sources = files(
  'src/main.cpp',
  'src/ui.cpp',
  'src/calculator.cpp',
  'src/config.cpp',
  'src/utils.cpp',
  'src/sha256.cpp'
)

deps = [dependency('glog', required: true), dependency('threads')]

executable('speakerbox', sources,
  include_directories: inc,
  dependencies: deps,
  install: false,
  build_by_default: true,
  install_dir: 'build')

# For debug: meson setup build --buildtype=debug
