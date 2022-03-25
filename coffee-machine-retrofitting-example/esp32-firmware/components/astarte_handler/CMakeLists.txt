set(astarte_handler_srcs "astarteHandler.c")

idf_component_register(SRCS "${astarte_handler_srcs}"
        INCLUDE_DIRS "include"
        REQUIRES astarte mbedtls coffee_machine_sensor)
