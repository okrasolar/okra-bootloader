mcu_inc = include_directories([
    '.',
])

if get_option('GIGADEVICE').enabled()
    mcu_files = files([
        'Bootloader_gd.cpp'
    ])
else
    mcu_files = files([
        'Bootloader_stm.cpp'
    ])
endif
