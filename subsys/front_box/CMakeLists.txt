if(CONFIG_NTURT_FRONT_BOX_API)

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_NTURT_FB_DASH_LEDS
    dash_leds.c
)

endif()
