project(
    'bar', 'cpp',
    version: '0.0.0',
    meson_version: '>= 0.59.0',
    default_options : [
        'cpp_std=c++23',
        'buildtype=release',
        'default_library=static'
    ],
)

compiler = meson.get_compiler('cpp')

cpp_args = []
cpp_link_args = []

if get_option('libcxx')
    cpp_args += ['-stdlib=libc++']
    cpp_link_args += ['-stdlib=libc++', '-lc++abi']
endif

inc_dirs = ['include']

src_files = files(
    'src/main.cxx',
)
executable(
    'bar',
    [src_files],
    include_directories: inc_dirs,
)
