if(CONFIG_NTURT_CMD)

target_sources(app PRIVATE cmd.c)

zephyr_linker_sources(DATA_SECTIONS iterable_sections.ld)

endif()
