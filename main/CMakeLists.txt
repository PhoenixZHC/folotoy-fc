if(FC_SIM_TEST)
    set(FC_ENTRY "../tests/fc_sim_main.c")
else()
    set(FC_ENTRY "game_main.c")
endif()
idf_component_register(
    SRCS ${FC_ENTRY} "gamepad.c" "gamepad_discovery.c" "gb_input.c"
         "gb_display.c" "gb_pixels.c" "gb_storage.c" "gb_saves.c"
         "ui_font.c" "gb_name.c" "gb_web.c" "gb_audio.c"
         "gb_audio_pacing.c" "gb_frame_pacing.c" "fc_port.c" "fc_rom.c" "fc_launch.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "gb_manager.html"
    REQUIRES bsp bluepad32 btstack nofrendo nvs_flash esp_lcd esp_timer esp_app_format
             esp_partition mbedtls spiffs esp_wifi esp_netif esp_event esp_http_server
)
target_compile_options(${COMPONENT_LIB} PRIVATE -O2)
option(FC_INPUT_TRACE "Trace decoded BLE and game-frame button masks" OFF)
if(FC_INPUT_TRACE)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE FC_INPUT_TRACE=1)
endif()
