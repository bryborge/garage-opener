project('garage_opener', 'c',
    version : '0.0.0',
    meson_version : '>=1.7.0',
    default_options : [
        'c_std=c11',
        'warning_level=2',
        'b_sanitize=address,undefined',
        'unity:support_double=true',
    ],
)

# Third-party libraries
iom361_dep = dependency('iom361_r4')

# The main program
executable('garage_opener', 'src/main.c',
    dependencies : iom361_dep,
)

# Tests
unity_dep = declare_dependency(
    dependencies : dependency('unity'),
    compile_args : '-DUNITY_INCLUDE_DOUBLE',
)
test_main = executable('test_main', 'tests/test_main.c',
    dependencies : [iom361_dep, unity_dep],
)
test('test_main', test_main)
