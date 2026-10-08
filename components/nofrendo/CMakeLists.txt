idf_component_register(
    SRCS "nofrendo.c" "nes/nes.c" "nes/cpu.c" "nes/ppu.c" "nes/apu.c"
         "nes/mem.c" "nes/mmc.c" "nes/input.c"
         "mappers/map000.c" "mappers/map001.c" "mappers/map002.c"
         "mappers/map003.c" "mappers/map004.c" "mappers/map007.c" "mappers/map023.c" "mappers/map066.c"
    INCLUDE_DIRS "." "nes"
)
target_compile_options(${COMPONENT_LIB} PRIVATE -O2 -Wno-error=format -Wno-error=unused-variable)
# CPU dispatch runs from flash, not an IRAM interrupt. Restore GCC's indexed
# switch dispatch instead of the ESP-IDF default comparison tree for 256 opcodes.
set_source_files_properties("nes/cpu.c" PROPERTIES COMPILE_OPTIONS "-fjump-tables;-ftree-switch-conversion")
