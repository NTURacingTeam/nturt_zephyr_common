if(CONFIG_NTURT_MSG)

target_sources(app PRIVATE msg.c)

zephyr_linker_sources(SECTIONS rx_init.ld)
zephyr_linker_sources(DATA_SECTIONS tx_init.ld)

target_sources_ifdef(CONFIG_NTURT_MSG_HB app PRIVATE node_mon.c)
target_sources_ifdef(CONFIG_NTURT_MSG_SDO_SRV app PRIVATE sdo_srv.c)
target_sources_ifdef(CONFIG_NTURT_MSG_SDO_CLI app PRIVATE sdo_cli.c)
target_sources_ifdef(CONFIG_NTURT_MSG_TIME app PRIVATE msg_time.c)

endif()
