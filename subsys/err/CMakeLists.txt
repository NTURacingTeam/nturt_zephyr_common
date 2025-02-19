if(CONFIG_NTURT_ERR)

zephyr_library()
zephyr_library_sources(
    err.c
)

zephyr_linker_sources(DATA_SECTIONS iterable_sections.ld)

endif()
