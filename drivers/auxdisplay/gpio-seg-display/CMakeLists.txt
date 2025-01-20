if(CONFIG_AUXDISPLAY_GPIO_SEG_DISPLAY)

zephyr_library()
zephyr_library_sources(
    gpio_seg_display.c
)

endif()
