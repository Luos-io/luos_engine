set(srcs "../../../../../engine/core/src/luos_engine.c"
    "../../../../../engine/core/src/luos_utils.c"
    "../../../../../engine/core/src/node.c"
    "../../../../../engine/core/src/profile_core.c"
    "../../../../../engine/core/src/pub_sub.c"
    "../../../../../engine/core/src/routing_table.c"
    "../../../../../engine/core/src/service.c"
    "../../../../../engine/core/src/stats.c"
    "../../../../../engine/core/src/streaming.c"
    "../../../../../engine/core/src/timestamp.c"
    "../../../../../engine/bootloader/bootloader_core.c"
    "../../../../../engine/HAL/ESP32/luos_hal.c"
    "../../../../../engine/IO/src/msg_alloc.c"
    "../../../../../engine/IO/src/luos_phy.c"
    "../../../../../engine/IO/src/luos_io.c"
    "../../../../../engine/IO/src/filter.c"
    "../../../src/port_manager.c"
    "../../../src/reception.c"
    "../../../src/robus.c"
    "../../../src/topic.c"
    "../../../src/transmission.c"
    "../../../HAL/ESP32/robus_hal.c")

set(inc "../../../../../engine/core/inc"
    "../../../../../engine/IO/inc"
    "../../../../../engine/OD"
    "../../../../../engine/HAL/ESP32"
    "../../../../../engine/bootloader"
    "../../../inc"
    "../../ESP32")

idf_component_register(SRCS ${srcs}
    INCLUDE_DIRS ${inc}
    LDFRAGMENTS "linker.lf")
