if(CONFIG_NTURT_REAR_BOX_API)

target_sources(app PRIVATE
    cmd.c
    ctrl.c
    states.c
)

endif()
