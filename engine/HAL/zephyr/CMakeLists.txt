if(CONFIG_LUOS)

zephyr_interface_library_named(LUOS_ENGINE)

zephyr_include_directories(
    ../../../network/robus/inc
    ../../../network/robus/zephyr
    ../../../engine/HAL/zephyr
    ../../../engine/core/inc
    ../../../engine/bootloader
    ../../../engine/OD
)

zephyr_library()
zephyr_library_sources(
    ../../../engine/core/src/luos_engine.c
    ../../../engine/core/src/luos_utils.c
    ../../../engine/core/src/profile_core.c
    ../../../engine/core/src/routing_table.c
    ../../../engine/core/src/streaming.c
    ../../../engine/core/src/timestamp.c
    ../../../engine/bootloader/bootloader_core.c
    ../../../engine/HAL/zephyr/luos_hal.c
)

zephyr_library_link_libraries(LUOS_ENGINE)
target_link_libraries(LUOS_ENGINE INTERFACE zephyr_interface)

endif()
