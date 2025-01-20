if(CONFIG_NTURT_REAR_BOX_API)

zephyr_library()
zephyr_library_sources(
    cmd.c
    ctrl.c
    states.c
)

endif()
