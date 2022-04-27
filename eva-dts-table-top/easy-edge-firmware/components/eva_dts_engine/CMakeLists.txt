
set (eva_dts_engine_srcs "eva_dts_engine.c")

idf_component_register (
    SRCS "${eva_dts_engine_srcs}"
    INCLUDE_DIRS "include"
)
