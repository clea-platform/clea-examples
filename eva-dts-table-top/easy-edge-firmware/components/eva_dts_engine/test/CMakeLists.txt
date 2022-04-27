#set(eva_dts_engine_tests_srcs "eva-dts-engine_test.c" )

idf_component_register(SRCS "eva-dts-engine_test.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES idf_test unity)
