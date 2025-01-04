if(CONFIG_NTURT_TM)

target_sources(app PRIVATE telemetry.c)

zephyr_linker_sources(DATA_SECTIONS iterable_sections.ld)

endif()
