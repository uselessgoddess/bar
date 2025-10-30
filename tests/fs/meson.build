test_inc = include_directories('../../include')

test_src = files(
    '../main.cxx',
    'fs.cxx',
)

fs_test = executable(
    'fs_test',
    test_src,
    dependencies: test_dep,
    include_directories: test_inc,
)

test(
    'fs',
    fs_test,
    workdir: meson.project_source_root(),
)
