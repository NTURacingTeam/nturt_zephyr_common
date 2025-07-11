if(CONFIG_NTURT_MSG)

zephyr_library()
zephyr_library_sources(
    aggregation.c
    msg.c
)

zephyr_library_sources_ifdef(CONFIG_NTURT_MSG_SHELL
    shell.c
)

endif()
