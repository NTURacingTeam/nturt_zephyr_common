if(CONFIG_INPUT_SENSOR_AXIS)

zephyr_library()
zephyr_library_sources(
    sensor_axis.c
)

zephyr_library_sources_ifdef(CONFIG_INPUT_SENSOR_AXIS_SETTINGS
    settings.c
)

zephyr_library_sources_ifdef(CONFIG_INPUT_SENSOR_AXIS_SHELL
    shell.c
)

endif()
