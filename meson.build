project(
  'bar',
  'cpp',
  version: '0.0.1',
  meson_version: '>= 0.59.0',
  default_options: ['cpp_std=c++23', 'buildtype=release', 'default_library=static'],
)

compiler = meson.get_compiler('cpp')

cpp_args = []
cpp_link_args = []

if get_option('libcxx')
  cpp_args += ['-stdlib=libc++']
  cpp_link_args += ['-stdlib=libc++', '-lc++abi']
endif

fmt = dependency('fmt', version: ['>=8.1.1'], fallback: ['fmt', 'fmt_dep'])
argparse = dependency('argparse')
barkeep = dependency('barkeep', required: get_option('barkeep'))

inc_dirs = ['include']

src_files = files(
  'src/main.cxx',
)

if barkeep.found()
    add_project_arguments('-DHAVE_BARKEEP', language: 'cpp')
    add_project_arguments('-DBARKEEP_ENABLE_FMT_FORMAT', language: 'cpp')
endif

executable(
  'bar',
  [src_files],
  dependencies: [fmt, argparse, barkeep],
  include_directories: inc_dirs,
)

catch2 = dependency(
  'catch2',
  default_options: ['tests=false'],
  fallback: ['catch2', 'catch2_dep'],
  required: get_option('tests'),
)
if catch2.found()
  subdir('tests')
endif

clangtidy = find_program('clang-tidy', required: false)

if clangtidy.found()
  run_target(
    'tidy',
    command: [
      clangtidy,
      '-checks=*,-fuchsia-default-arguments',
      '-p', meson.project_build_root(),
    ]
    + src_files,
  )
endif
