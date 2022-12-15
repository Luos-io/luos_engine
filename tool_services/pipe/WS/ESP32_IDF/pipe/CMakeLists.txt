set(srcs "../../../pipe.c"
    "../pipe_com.c"
    "../mongoose/mongoose.c")

set(inc "../../ESP32_IDF"
    "../../../../pipe"
    "../mongoose"
    "../mongoose/src")

idf_component_register(SRCS ${srcs}
    INCLUDE_DIRS ${inc}
    REQUIRES luos_engine nvs_flash)
