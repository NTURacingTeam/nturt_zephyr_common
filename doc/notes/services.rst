.. _notes_services:

===============
Zephyr Services
===============

Logging
=======

Zephyr provides a comprehensive logging system that provides various logging
backends (i.e., where these log messages will be outputed to) such as console or
file system.

File system backend enabled by Kconfig option `CONFIG_LOG_BACKEND_FS
<https://docs.zephyrproject.org/3.6.0/kconfig.html#CONFIG_LOG_BACKEND_FS>`_ is
particularly handy since it will only log message only after the file system is
mounted.

Tracing
=======

Zephyr provides a tracing system that allows you to trace the execution of
kernel objects such as threads, work queues and mutexes, etc. Here we use
`Segger SystemView <https://www.segger.com/products/development-tools/systemview/>`_
as the tracing backend.

Custom SystemView Description
-----------------------------

Though SystemView provides description table for Zephyr, it's not complete as
mentioned in the `tracing subsystem documentation
<https://docs.zephyrproject.org/3.7.0/services/tracing/index.html#segger-systemview-support>`_.
However, to add the proper description for Zephyr, the correct description file,
i.e. ``zephyr/supsys/tracing/sysview/SYSVIEW_Zephyr.txt``, should be placed at
``/opt/SEGGER/SystemView/Description`` [#]_ for Linux and Mac OS systems, or at
``C:\Program Files\SEGGER\SystemView\Description`` for Windows systems.

Interrupt Service Routine (ISR) Number
--------------------------------------

When ISR is executed, it does not have a name that is easily recognizable.
Instead a number is used to identify the ISR. For cortex-M series, the number is
the first nine bits of Interrupt Control and State Register (ICSR) register of
the System Control Block (SCB) in the CPU [#]_. Which corresponds to the number
of the ISR to be called in the interrupt vector table.

For STM32 microcontrollers, this table is listed in Interrupt and exception
vectors in the Nested vectored interrupt controller (NVIC) section in the
reference manual. For example, if the ISR number is 102, it corresponds to
address 0x198 (102*4 in decimal) and for STM32G4 series, it is fired from FDCAN2_IT0 line.

Reference
---------

.. [#] `zephyr sysview usage
  <https://blog.ekko.cool/zephyr%20sysview%20%E4%BD%BF%E7%94%A8?locale=zh>`_
.. [#] `sysview_get_interrupt() source code that determines the ISR number
  <https://github.com/zephyrproject-rtos/zephyr/blob/v3.7.0/subsys/tracing/sysview/sysview.c#L24>_`

LittleFS
========

LittleFS support both non-volatile memory (NVM) such as internal flash or
external SPI flash and block device such as SD cards or USB drives. However,
since there is little to no example for the latter, some quirks are worth noting
here, and a SD card block device is used here as an example.

Though LittleFS provides `device tree bindings
<https://docs.zephyrproject.org/3.6.0/build/dts/api/bindings/fs/zephyr%2Cfstab%2Clittlefs.html#dtbinding-zephyr-fstab-littlefs>`_
for configuring the file system, it is mainly designed for NVM. For block
devices, LittleFS will determine the block size when mounting the device, and
set other parameters such as the read and program size the same as the block
size and lookhead size four times the block size [#]_. Since it does not know
how big the block size will be, it simply uses :c:func:`melloc` to allocate the
read, program, and lookhead buffers for the block device [#]_, so be sure to
enable :c:func:`melloc` and set the heap size to at least six times the size of
the block size.

Additionally, LittleFS uses :c:func:`k_heap_alloc` for allocating file caches
[#]_ using a memory pool controlled by Kconfig option
`CONFIG_FS_LITTLEFS_CACHE_SIZE
<https://docs.zephyrproject.org/latest/kconfig.html#CONFIG_FS_LITTLEFS_CACHE_SIZE>`_,
so also make sure to set it to values greater than block size.

Since the automount feature is not available for block devices, they must be
mounted manually. The following code snippet shows how to do so:

.. code-block:: c

  static struct fs_littlefs lfsfs;
  static struct fs_mount_t mp = {
      .type = FS_LITTLEFS,
      .fs_data = &lfsfs,
      .flags = FS_MOUNT_FLAG_USE_DISK_ACCESS,
      .storage_dev = CONFIG_SDMMC_VOLUME_NAME,
      .mnt_point = "/" CONFIG_SDMMC_VOLUME_NAME ":",
  };

  fs_mount(&mp);

Reference
---------

.. [#] `LittleFS littlefs_init_cfg() source code
  <https://github.com/zephyrproject-rtos/zephyr/blob/v3.6.0/subsys/fs/littlefs_fs.c#L822>`_
  that initializes read, program, and lookhead buffer sizes
.. [#] `LittleFS lfs_init() source code
  <https://github.com/zephyrproject-rtos/littlefs/blob/zephyr/lfs.c#L4114>`_
  that allocate read, program, and lookhead buffer
.. [#] `LittleFS littlefs_open() source code
  <https://github.com/zephyrproject-rtos/zephyr/blob/v3.6.0/subsys/fs/littlefs_fs.c#L302>`_
  that allocate file cache
