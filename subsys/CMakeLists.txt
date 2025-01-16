add_subdirectory(cmd)
add_subdirectory(err)
add_subdirectory(msg)
add_subdirectory(sensors)
add_subdirectory(sys)
add_subdirectory(telemetry)

add_subdirectory(front_box)
add_subdirectory(rear_box)

target_sources_ifdef(CONFIG_NTURT app PRIVATE
    util.c
)
