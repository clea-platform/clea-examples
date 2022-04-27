
set (astarte_handler_srcs "astarte_handler.c")

idf_component_register (
    SRCS "${astarte_handler_srcs}"
    INCLUDE_DIRS "include"
    REQUIRES astarte mbedtls
)
