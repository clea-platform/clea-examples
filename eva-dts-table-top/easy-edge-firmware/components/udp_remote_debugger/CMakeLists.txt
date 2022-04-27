
set(udp_remote_debugger_srcs "udp_remote_debugger.c")

idf_component_register (
    SRCS "${udp_remote_debugger_srcs}"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash
)
