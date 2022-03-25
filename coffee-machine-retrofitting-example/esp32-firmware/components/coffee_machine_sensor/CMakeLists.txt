set(coffee_sensor_srcs coffee_machine_sensor.c)

idf_component_register(SRCS "${coffee_sensor_srcs}"
        INCLUDE_DIRS "include")
