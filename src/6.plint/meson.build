project('plint', 'c')
inc = include_directories('printf')
add_project_arguments('-Wall', '-O2', '-fno-omit-frame-pointer', '-ggdb' ,'-march=rv64gcvxtheadc', '-mabi=lp64d', '-mtune=c906', '-nostartfiles', '-MD', '-mcmodel=medany', '-ffreestanding', '-fno-common' ,'-nostdlib', '-mno-relax', '-I.','-I inc', language: 'c')

linker_script = join_paths(meson.current_source_dir(), 'link.ld')
add_project_link_arguments('-T', linker_script ,'-nostdlib', '-nostartfiles', language: 'c')
exe = executable('timer', ['plint.c','clint.c','clk.c', 'common.c', 'gpio.c', 'interrupt.c', 'main.c', 'printf.c', 'uart.c', 'timer.c', 'start.S','trap.S'],include_directories:[inc])

hex = custom_target('Image',
  build_by_default: true,
  input : exe,
  output : 'Image',
  command : [meson.get_external_property('objcopy'), '-O', 'binary','@INPUT@','@OUTPUT@'],
)
