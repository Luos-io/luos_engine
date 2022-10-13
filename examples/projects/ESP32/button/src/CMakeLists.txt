#Edit following two lines to set component requirements (see docs)

 set(srcs    "main.c")

 set(inc     ".")

 idf_component_register( SRCS ${srcs}
                           INCLUDE_DIRS ${inc}
                           REQUIRES luos_engine button)

#idf_component_register(SRCS "main.c")



