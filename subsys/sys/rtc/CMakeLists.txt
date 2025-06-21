if(CONFIG_NTURT_RTC)

zephyr_library()
zephyr_library_sources(
    rtc.c
)

endif()
