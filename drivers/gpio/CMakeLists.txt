zephyr_library()

zephyr_library_sources_ifdef(CONFIG_GPIO_74HC595 gpio_74hc595.c)
