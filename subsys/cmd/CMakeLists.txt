if(CONFIG_NTURT_CMD)

zephyr_library()
zephyr_library_sources(
    cmd.c
)

zephyr_linker_sources(DATA_SECTIONS
    iterable_sections.ld
)

endif()
