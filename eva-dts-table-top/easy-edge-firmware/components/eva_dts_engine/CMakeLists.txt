set(astarte_handler_srcs "eva_dts_engine.c")

idf_component_register(SRCS "${astarte_handler_srcs}"
        INCLUDE_DIRS "include")
