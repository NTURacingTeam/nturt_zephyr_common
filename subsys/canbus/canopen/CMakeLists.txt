if(CONFIG_NTURT_CANOPEN)

zephyr_library()
zephyr_library_sources(
    canopen.c
    emcy.c
)

zephyr_library_sources_ifdef(CONFIG_NTURT_CANOPEN_TIME
    time.c
)

zephyr_library_sources_ifdef(CONFIG_NTURT_CANOPEN_STORAGE
    storage.c
)

zephyr_linker_sources(DATA_SECTIONS
    iterable_sections.ld
)

# target_sources_ifdef(CONFIG_NTURT_MSG_HB app PRIVATE node_mon.c)
# target_sources_ifdef(CONFIG_NTURT_MSG_SDO_SRV app PRIVATE sdo_srv.c)
# target_sources_ifdef(CONFIG_NTURT_MSG_SDO_CLI app PRIVATE sdo_cli.c)

endif()
