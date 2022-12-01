set(srcs "gate.c"
    "pipe_link.c"
    "data_manager.c"
    "TinyJSON/bootloader_ex.c"
    "TinyJSON/convert.c"
    "TinyJSON/tiny-json.c")

set(inc "."
    "TinyJSON")

idf_component_register(SRCS ${srcs}
    INCLUDE_DIRS ${inc}
    REQUIRES luos_engine)
