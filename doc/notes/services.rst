.. _notes_services:

===============
Zephyr Services
===============

Data Passing
============

Zephyr provides various ways to pass data between threads and ISRs, the
following tables summerizes the differences between them:

Kernel Services for Data Passing
--------------------------------

Generally, both the producing and consuming sides of the data passing methods
provided by the kernel can be accross multiple contexts, i.e., threads and ISRs,
as the them utilizes locks in both the producing and consuming sides. In
addition, since this methods are also kernel objects, thay have the same
functionalites such as `object cores
<https://docs.zephyrproject.org/4.0.0/kernel/object_cores/index.html>`_, `object
tracing
<https://docs.zephyrproject.org/4.0.0/services/tracing/index.html#object-tracking>`_,
etc. as other kernel objects.

.. table::
   :widths: 15 10 15 15 20

   +---------------+-----------+--------------+----------------+-----------------------+
   |    Object     | Data size | Data storage | Data passed by |       Features        |
   +===============+===========+==============+================+=======================+
   | FIFO/LIFO     | Arbitrary | User managed | Pointer        | Intrusive structure   |
   +---------------+-----------+--------------+----------------+-----------------------+
   | Stack         | Word      | Array        | Copy           | For passing pointers  |
   +---------------+-----------+--------------+----------------+-----------------------+
   | Pipe          | Byte      | Ring buffer  | Copy           | For passing raw bytes |
   +---------------+-----------+--------------+----------------+-----------------------+
   | Message Queue | Fixed     | Ring buffer  | Copy           | For passing structs   |
   +---------------+-----------+--------------+----------------+-----------------------+
   | Mailbox       | Arbitrary | User managed | Pointer        | Destined transfer     |
   +---------------+-----------+--------------+----------------+-----------------------+

Refer to the `data passing section of the kernel service documentation
<https://docs.zephyrproject.org/latest/kernel/services/index.html#data-passing>`_
for more details.

Kernel Data Structures for Data Passing
---------------------------------------

Two data structures provided by the kernel are specifically designed for data
passing, namely single producer single consumer (SPSC) and multiple producer
single consumer (MPSC) packet buffers. And there are also lockless versions of
them that is intrusive and requires the user to manage the data storage.

Note that as the name suggests, SPSC is designed for passing data between a
single producing context and a single consuming context, and MPSC is designed
for multiple producing contexts and a single consuming context, unlike the
kernel services for data passing that allows multiple producing and consuming
contexts.

.. table::
   :widths: 15 10 15 15 20

   +--------------------+-----------+--------------+----------------+-------------------------+
   |       Object       | Data size | Data storage | Data passed by |        Features         |
   +====================+===========+==============+================+=========================+
   | SPSC/MPSC          | Arbitrary | Ring Buffer  | Copy           | Arbitrary-sized packets |
   +--------------------+-----------+--------------+----------------+-------------------------+
   | Lockless SPSC/MPSC | Arbitrary | User managed | Pointer        | Lockless                |
   +--------------------+-----------+--------------+----------------+-------------------------+
   | Winstream          | Byte      | Ring buffer  | Copy           | Lockless SPSC           |
   +--------------------+-----------+--------------+----------------+-------------------------+

Refer to the `kernel data structures
<https://docs.zephyrproject.org/4.0.0/kernel/data_structures/index.html>`_ for
more details.

.. note::

   The Winstream data structure is not documented in the kernel data structures
   documentation, but the API documentation is available in `the file reference
   <https://docs.zephyrproject.org/4.0.0/doxygen/html/winstream_8h.html>`_.

OS Services for Data Passing
----------------------------

Data passing methods provided by OS services are built on top of the kernel ones
and have additional features such as sublisher/subscriber model, callbacks, etc.

.. table::
   :widths: 15 10 15 15 20

   +--------+-----------+--------------+----------------+-----------+
   | Object | Data size | Data storage | Data passed by | Features  |
   +========+===========+==============+================+===========+
   | Zbus   | Fixed     | Queue        | Pointer/Copy   | Callbacks |
   +--------+-----------+--------------+----------------+-----------+

Refer to their respective documentations for more details.

Pooled Parallel Preemptible Priority-based Work Queues (P4WQ)
=============================================================

Aside from the `kernel work queue
<https://docs.zephyrproject.org/4.0.0/kernel/services/threads/workqueue.html>`_
that runs only in one thread, Zephyr provides a more advanced work queue system
called P4WQ that provides parallel execution of works using thread pool based on
its priority, with the tradeoff not having delayable works and thread pool
control API. Additionally, it is not documented in the Zephyr documentation, and
its API documentation is only available in `its file reference
<https://docs.zephyrproject.org/4.0.0/doxygen/html/p4wq_8h.html>`_. P4WQ can be
enabled by Kconfig option ``CONFIG_SCHED_DEADLINE``.

:c:struct:`k_p4wq_work` is used to define a work item, where
:c:member:`k_p4wq_work.priority` and :c:member:`k_p4wq_work.deadline` are what
the priority and the deadline of the thread will be when the work is executed.
This however means that when all threads are busy, newly submitted works with
higher priority will not preempt the lower priority works that are already being
executed [#]_.

.. note::

   Thread deadline in Zephyr is only considered when scheduling threads with the
   same priority using `earliest deadline first scheduling
   <https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling>`_ [#]_.

References
----------

.. [#] `p4wq source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/lib/os/p4wq.c#L283>`_
   that breaks priority guarantee
.. [#] `k_thread_deadline_set()
   <https://docs.zephyrproject.org/4.0.0/doxygen/html/group__thread__apis.html#gad887f16c1dd6f3247682a83beb22d1ce>`_
   API documentation

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

.. [#] `Zephyr sysview usage
   <https://blog.ekko.cool/zephyr%20sysview%20%E4%BD%BF%E7%94%A8?locale=zh>`_
.. [#] `sysview_get_interrupt() source code that determines the ISR number
   <https://github.com/zephyrproject-rtos/zephyr/blob/v3.7.0/subsys/tracing/sysview/sysview.c#L24>`_

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

.. _notes_services_sensing_subsystem:

Storage Systems
===============

Aside from file systems, Zephyr also provides lower-level storage systems that
are designed to store data on the limited internal flash, such as `Flash
Circular Buffer (FCB)
<https://docs.zephyrproject.org/4.1.0/services/storage/fcb/fcb.html>`_ that
stores data in FIFO manner, as well as `Non-Volatile Storage (NVS)
<https://docs.zephyrproject.org/4.1.0/services/storage/nvs/nvs.html>`_ and
`Zephyr Memory Storage (ZMS)
<https://docs.zephyrproject.org/latest/services/storage/zms/zms.html>`_ that
store using key-value pairs (refer to the `documentation of ZMS
<https://docs.zephyrproject.org/4.1.0/services/storage/zms/zms.html#zms-and-other-storage-systems-in-zephyr>`_
for detailed comparison between them).

.. note::

   NVS stores the flash page size (the unit of an erase operation) in a
   :c:type:`uint16_t` [#]_, so it can only support flash page size up to 32KB
   (page size is typically in power of 2 and :c:type:`uint16_t` is at most 65535
   so 32KB is the maximum). Though ZMS does not have such limit, since it have
   to go through the entire all pages for every write operation [#]_, it is not
   suitable for large total size. And since flash with large page sizes such as
   STM32F4 and STM32H7 series will inevitably have large total size, both
   technologies are not suitable for such use cases. Instead, external SPI flash
   have to be used.

Settings
-------

Zephyr provides a `settings system
<https://docs.zephyrproject.org/4.1.0/services/storage/settings/index.html>`_
that built on top of the storage or file systems to store and load settings and
its backend can be selected by Kconfig option ``CONFIG_SETTINGS_BACKEND``.

.. note::

   For FCB, NVS, and ZMS backends, the settings system will automatically chose
   ``storage_partition`` if ``zephyr,settings_partition`` is not chosen in the
   device tree [#]_ [#]_ [#]_.


References
----------

.. [#] `:c:struct:`nvs_fs` source code <https://github.com/zephyrproject-rtos/zephyr/blob/v4.1.0/include/zephyr/fs/nvs.h#L51>`_
.. [#] `ZMS documentation
   <https://docs.zephyrproject.org/4.1.0/services/storage/zms/zms.html#zms-id-data-write>`_
.. [#] `Settings FCB backend source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.1.0/subsys/settings/src/settings_fcb.c#L23>`_
.. [#] `Settings NVM backend source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.1.0/subsys/settings/src/settings_nvs.c#L23>`_
.. [#] `Settings ZMS backend source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/main/subsys/settings/src/settings_zms.c#L24>`_

Sensing Subsystem
=================

The `sensing subsystem
<https://docs.zephyrproject.org/4.0.0/services/sensing/index.html>`_ provides a
high level of accessing sensors, such as scheduling sampling for multiple
clients that requests data at different rates and resoultons (since currently
most sensor drivers are designed for single client only [#]_), and fusing
multiple sensors to provide a new kind of data (e.g. fusing two IMUs on the lid
and the base of a foldable phone to calculate the hinge angle).

However, dispite `being merged to the mainline in October 2023
<https://github.com/zephyrproject-rtos/zephyr/pull/64478>`_, the sensing
subsystem is still only a minimum viable product as of Zephyr 4.0.0 lacking some
important features such as batch reading and error reporting [#]_. And currently
seemed to be not in active development anymore.

Yet it is still a good starting point for managing multiple sensors with a
single interface.

Some issues of the sensing subsystem should be considered or addressed before
being adopted:

Duplicated Sensor Data Types
----------------------------

In the documentation of sensing subsystem,
:c:struct:`sensing_sensor_value_q31` is the data type for sensor data, which is
almost the same as :c:struct:`sensor_q31_data` used for the sensor driver. The
only difference is that the timestamp is in micro seconds for the former and in
nano seconds for the latter [#]_ [#]_.

This is probably due to :c:struct:`sensor_q31_data` being introduced later than
the initial proposal of the sensing subsystem. However, since
:c:struct:`sensor_q31_data` is widely adopted by the sensor driver, it should be
used as the sensor data type in the sensing subsystem instead of
:c:struct:`sensing_sensor_value_q31`.

Use of HID Sensor Type (instead of existing :c:enum:`sensor_channel`)
---------------------------------------------------------------------

Intel (the author of the sensor subsystem) uses HID sensor types instead of the
existing :c:enum:`sensor_channel` of sensor types from CHRE (context hub runtime
environmrnt) since they claimed that it is the only cross-OS standard for sensor
types [#]_. However, they did not implement any other part of the HID standard,
making the choice of using HID sensor type that's incompatible with the existing
:c:enum:`sensor_channel` existing sensor drivers depends on some what abrubtly.
Additionally, the sensing subsystem configures the sensor using
`sensing_set_config()
<https://docs.zephyrproject.org/4.0.0/doxygen/html/group__sensing__api.html#ga46591a2d29f5b5e160a72bbe289884ab>`_
that requires :c:enum:`sensor_channel` as the parameter as mentioned in the
next section, it is better to stick to :c:enum:`sensor_channel` for sensor
types.

Confusing Device API
--------------------

During development, the sensing subsystem sensor device API is required to be 
the same as the sensor driver :c:struct:`sensor_driver_api` as mentioned in
`this RFC <https://github.com/zephyrproject-rtos/zephyr/issues/62223>`_. As as
result, when setting required interval and senstivity using
`sensing_set_config()
<https://docs.zephyrproject.org/4.0.0/doxygen/html/group__sensing__api.html#ga46591a2d29f5b5e160a72bbe289884ab>`_,
it sets ``SENSOR_ATTR_SAMPLING_FREQUENCY`` and ``SENSOR_ATTR_HYSTERESIS``
attributes, respectively, of the channel corresponding to that HID sensor type
via `sensor_attr_set()
<https://docs.zephyrproject.org/4.0.0/doxygen/html/group__sensor__interface.html#gafbf65226a227e9f8824908bc38e336f5>`_ [#]_ [#]_,
which is somewhat confusing, but have to bare in mind when developing sensor
subsystem drivers.

Sensor Scheduling
-----------------

Sample scheduling is done internally by first setting
``SENSOR_ATTR_SAMPLING_FREQUENCY`` sensor attribute based on the minimum
requested sampling intervals of all clients, and then downsample the data via a
timer that only passes the data to the clients when the time elapsed from the
last sample is greater than the period [#]_. Which means that the actual
sampling interval will be longer and unpredictable if the underlying sensor
driver sampling jitters. Both of which are not ideal for real-time applications.

A better approach would be to set the sensor driver sampling rate based on the
greatest common factor of all clients' requested sampling intervals with some
kind of tolerence for the jitter.

References
----------

.. [#] `Sensor fetch and get
   <https://docs.zephyrproject.org/4.0.0/hardware/peripherals/sensor/fetch_and_get.html>`_
   warning
.. [#] `Sensing subsystem dispatch_task() source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/subsys/sensing/dispatch.c#L102>`_
   that ignores cqe result codes
.. [#] `Definition of sensing_sensor_value_q31
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/include/zephyr/sensing/sensing_datatypes.h#L116>`_
.. [#] `Definition of sensor_q31_data
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/include/zephyr/drivers/sensor_data_types.h#L92>`_
.. [#] `Sensing Subsystem Design Discussion
   <https://github.com/zephyrproject-rtos/zephyr/files/12561847/Sensing.Subsystem.Design.-.Zephyr.pdf>`_,
   slide 13
.. [#] `Sensing subsystem send_data_to_clients() source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/subsys/sensing/dispatch.c#L38>`_
   that dispatches sensor data to clients
.. [#] `Sensing subsystem set_arbitrate_interval() source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/subsys/sensing/sensor_mgmt.c#L92>`_
.. [#] `Sensing subsystem set_arbitrate_sensitivity() source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/subsys/sensing/sensor_mgmt.c#L170>`_
