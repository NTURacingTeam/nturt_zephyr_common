zephyr_library()

zephyr_library_sources_ifdef(CONFIG_LED_STRIP_GPIO led_strip_gpio.c)
