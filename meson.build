project(
    'edamame-bootloader',
    'c',
    'cpp',
    version: '0.0.4',
    default_options:  [ 'c_std=c11', 'cpp_std=gnu++11' ]
)

assert(meson.is_cross_build(), 'This project can only be built in a cross build environment.')

# Import binary helpers
stinfo       = find_program('st-info')
stflash      = find_program('st-flash')
python       = find_program('python3', required: false)
objcopy      = '@0@'.format(find_program('objcopy').path())
size         = '@0@'.format(find_program('size').path())

# Build time defines
option_defines = []
if get_option('COPYBINARY').enabled()
    option_defines += '-DCOPYBINARY'
endif

system_files = files([ 'src/System_stm32.cpp' ])

# Startup and system files
if (host_machine.cpu() == 'cortex-m3')
    system_files += files([
        'src/stm32f1/startup.s',
        'src/stm32f1/system.c',
    ])
    system_inc = include_directories([
        'CMSIS_STM32F1/Include',
        'CMSIS_STM32F1/Device/ST/STM32F1xx/Include'
    ])
    link_args = '-Wl,-T,@0@/@1@'.format(meson.current_source_dir(), 'src/stm32f1/linker.ld')
elif (host_machine.cpu() == 'cortex-m4')
    system_files += files([
        'src/stm32f3/startup.s',
        'src/stm32f3/system.c',
    ])
    system_inc = include_directories([
        'CMSIS_STM32F3/Include',
        'CMSIS_STM32F3/Device/ST/STM32F3xx/Include'
    ])
    link_args = '-Wl,-T,@0@/@1@'.format(meson.current_source_dir(), 'src/stm32f3/linker.ld')
endif

c_args = [ option_defines, '-Os' ]

# Add src dependancies
src_inc   = [ 'src/' ]
src_files = [ 'src/Bootloader.cpp' ]

# Generate elf file for MCU
main_elf = executable(
    'main',
    [ system_files, src_files, 'src/main.cpp' ],
    name_suffix         : 'elf',
    include_directories : [ system_inc, src_inc ],
    cpp_args            : [ c_args ],
    c_args              : [ c_args ],
    link_args           : [ link_args ]
)

message(host_machine.cpu())

if (meson.is_subproject() != true)
    # Build custom targets MCU
    main_bin = custom_target(
        'main_bin',
        output           : [ 'main.bin' ],
        build_by_default : true,
        command          : [ objcopy, '-O', 'binary', '-S', 'main.elf', 'main.bin' ],
        depends          : [ main_elf ]
    )
    main_hex = custom_target(
        'main_hex',
        output           : [ 'main.hex' ],
        build_by_default : false,
        command          : [ objcopy, '-O', 'ihex', 'main.elf', 'main.hex' ],
        depends          : [ main_elf ]
    )

    # Add CppUTest dependancy
    cpputest     = subproject('cpputest')
    cpputest_dep = cpputest.get_variable('cpputest_dep')

    # Build native test executable
    subdir('test')
    test_files = get_variable('test_files')
    main_test = executable(
        'tests',
        [ src_files, test_files ],
        include_directories : [ system_inc, src_inc ],
        dependencies        : [ cpputest_dep ],
        c_args: option_defines,
        cpp_args: option_defines,
        native              : true,
        build_by_default    : false
    )
    test('Bootloader test', main_test)

    # Custom run commands
    run_target('erase',             command: [ stflash,  'erase' ])
    run_target('lint',              command: [ python, '.clang-format.py', '-r',       '-e', 'src', 'src' ])
    run_target('fix',               command: [ python, '.clang-format.py', '-r', '-s', '-e', 'src', 'src' ])
    run_target('flash',             command: [ stflash, '--reset', 'write', main_bin, '0x08000000' ], depends: main_bin)
    run_target('size',              command: [ size, main_hex ], depends: main_hex)
endif
