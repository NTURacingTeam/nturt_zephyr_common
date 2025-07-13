add_subdirectory(canbus)
add_subdirectory(cmd)
add_subdirectory(err)
add_subdirectory(msg)
add_subdirectory(sensors)
add_subdirectory(sensing)
add_subdirectory(sys)
add_subdirectory(telemetry)

if(CONFIG_NTURT)

zephyr_library()
zephyr_library_sources(
    util.c
)

endif()
