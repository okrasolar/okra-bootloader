project(
    'edamame-bootloader',
    'c',
    'cpp',
    version: '0.0.3',
    subproject_dir: 'dep',
    default_options:  [ 'c_std=c11', 'cpp_std=gnu++11' ]
)

assert(meson.is_cross_build(), 'This project can only be built in a cross build environment.')

# Import binary helpers
stinfo       = find_program('st-info')
stflash      = find_program('st-flash')
python       = find_program('python3', required: false)
objcopy      = '@0@'.format(find_program('objcopy').path())
size         = '@0@'.format(find_program('size').path())

# Startup and system files
system_files = files([
    'startup.s',
    'system.c'
])
system_inc = include_directories([
    'CMSIS/Include',
    'CMSIS/Device/ST/STM32F1xx/Include'
])
link_args = '-Wl,-T,@0@/@1@'.format(meson.current_source_dir(), 'linker.ld')

# Add src dependancies
subdir('src')
mcu_inc   = get_variable('mcu_inc')
mcu_files = get_variable('mcu_files')

# Generate elf file for MCU
main_elf = executable(
    'main',
    [ system_files, mcu_files ],
    name_suffix         : 'elf',
    include_directories : [ system_inc, mcu_inc ],
    link_args           : [ link_args]
)

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

# Custom run commands
run_target('erase',             command: [ stflash,  'erase' ])
run_target('lint',              command: [ python, '.clang-format.py', '-r',       '-e', 'src', 'src' ])
run_target('fix',               command: [ python, '.clang-format.py', '-r', '-s', '-e', 'src', 'src' ])
run_target('flash',             command: [ stflash, '--reset', 'write', main_bin, '0x08000000' ], depends: main_bin)
run_target('size',              command: [ size, main_hex ], depends: main_hex)