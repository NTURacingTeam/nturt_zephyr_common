#!/bin/bash

west twister --device-testing --hardware-map hardware_map.yaml \
    --fixture fixture_sdhc \
    --fixture fixture_display \
    --test sample.basic.helloworld \
    --test sample.filesystem.fat_fs \
    --test drivers.disk.stm32_sdhc \
    --test sample.usb.cdc-acm \
    --test sample.usb.mass_ram_fat

    # --test sample.display.builtin \
