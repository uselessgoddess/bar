test_inc = include_directories('../../include')

test_src = files(
    '../main.cxx',
    'io.cxx',
)

io_test = executable(
    'io_test',
    test_src,
    dependencies: test_dep,
    include_directories: test_inc,
)

test(
    'io',
    io_test,
    workdir: meson.project_source_root(),
)
