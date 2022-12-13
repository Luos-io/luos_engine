set(srcs "../../../pipe.c"
    "../pipe_com.c")

set(inc "../../ESP32_IDF"
    "../../../../pipe")

idf_component_register(SRCS ${srcs}
    INCLUDE_DIRS ${inc}
    REQUIRES luos_engine)
