if(CONFIG_LED_STRIP_GPIO)

zephyr_library()

zephyr_library_sources(
    led_strip_gpio.c
)

endif()
