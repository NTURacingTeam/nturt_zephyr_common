zephyr_library()

zephyr_library_sources_ifdef(CONFIG_AMT21 amt21.c)
