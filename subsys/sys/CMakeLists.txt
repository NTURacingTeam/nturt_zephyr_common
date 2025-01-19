if(CONFIG_NTURT_SYS)

target_sources(app PRIVATE
    sys.c
)

target_sources_ifdef(CONFIG_NTURT_SYS_FS app PRIVATE
    fs.c
)

target_sources_ifdef(CONFIG_NTURT_SYS_RTC app PRIVATE
    rtc.c
)

endif()
