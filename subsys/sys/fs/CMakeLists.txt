if(CONFIG_NTURT_SYS_FS)

zephyr_library()
zephyr_library_link_libraries(FS)
zephyr_library_sources(
    fs.c
)

endif()
