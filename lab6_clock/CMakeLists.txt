add_executable(lab6.elf main.c clockControl.c clockDisplay.c)
target_link_libraries(lab6.elf ${330_LIBS} touchscreen interrupts intervalTimer buttons_switches)
set_target_properties(lab6.elf PROPERTIES LINKER_LANGUAGE CXX)