if(CONFIG_NTURT_TM)

zephyr_library()
zephyr_library_sources(
    telemetry.c
)

zephyr_linker_sources(DATA_SECTIONS iterable_sections.ld)

endif()
