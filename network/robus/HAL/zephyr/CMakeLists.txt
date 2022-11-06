if(CONFIG_LUOS)

zephyr_interface_library_named(LUOS_ROBUS)

zephyr_include_directories(
    ../../inc
    ../zephyr
    ../../../../engine/HAL/zephyr
)

zephyr_library()
zephyr_library_sources(
    ../../src/msg_alloc.c
    ../../src/port_manager.c
    ../../src/reception.c
    ../../src/robus.c
    ../../src/topic.c
    ../../src/transmission.c
    ../../HAL/zephyr/robus_hal.c
)

zephyr_library_link_libraries(LUOS_ROBUS)
target_link_libraries(LUOS_ROBUS INTERFACE zephyr_interface)

endif()
