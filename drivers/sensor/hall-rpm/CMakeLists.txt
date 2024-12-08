zephyr_library()

zephyr_library_sources_ifdef(CONFIG_HALL_RPM hall_rpm.c)
