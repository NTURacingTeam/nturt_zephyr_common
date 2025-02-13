if(CONFIG_NTURT_MSG)

zephyr_library()
zephyr_library_sources(
    aggregation.c
    msg.c
)

endif()
