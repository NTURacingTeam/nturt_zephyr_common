if(CONFIG_NTURT_SYS)

add_subdirectory(fs)
add_subdirectory(rtc)

zephyr_library()
zephyr_library_sources(
    sys.c
)

endif()
