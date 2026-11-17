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
math_dep = meson.get_compiler('c').find_library('m', required : false)

# Modules
garage_inc = include_directories('include/garage_opener')
garage_lib = static_library('garage', 'src/sensor.c', 'src/fsm.c',
    include_directories : garage_inc,
    dependencies : [iom361_dep, math_dep],
)
garage_dep = declare_dependency(
    link_with : garage_lib,
    include_directories : garage_inc,
    dependencies : [iom361_dep, math_dep],
)

# The main program
garage_opener = executable('garage_opener', 'src/main.c',
    dependencies : garage_dep,
)

# Tests
unity_dep = declare_dependency(
    dependencies : dependency('unity'),
    compile_args : '-DUNITY_INCLUDE_DOUBLE',
)
test_sensor = executable('test_sensor', 'tests/test_sensor.c',
    dependencies : [garage_dep, unity_dep],
)
test_fsm = executable('test_fsm', 'tests/test_fsm.c',
    dependencies : [garage_dep, unity_dep],
)
test('sensor', test_sensor)
test('fsm', test_fsm)

# Scenario tests: replay each scenario and compare the output with its
# expected output.
run_scenario = find_program('tests/run_scenario.sh')
foreach name : [
    'normal',
    'thresholds',
    'freeze_guard',
    'unexpected_input',
    'midnight',
]
    test('scenario ' + name, run_scenario,
        args : [
            garage_opener,
            files('scenarios' / name + '.txt'),
            files('scenarios' / 'expected' / name + '_output.txt'),
        ],
    )
endforeach
