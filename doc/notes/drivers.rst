.. _notes_drivers:

==============
Zephyr Drivers
==============

Device Initialization
=====================

Each device are initialized at boot time by their initialization functions with
`different initialization levels and priorities
<https://docs.zephyrproject.org/4.0.0/kernel/drivers/index.html#initialization-levels>`_.
The initialization functions with the same level and priorities are called in
their dependency order. And devices that depend on other devices cannot be
initialized before the devices they depend on are initialized.

Device Dependency
-----------------

The dependency of each node in the device tree is determined by:

- The child is dependent on the parent node.
- The node that references another node (using phandle) is dependent on the
  referenced node.

And the final order is listed in
``zephyr/include/generated/zephyr/devicetree_generated.h`` of the build
directory [#]_.

References
----------

.. [#] `Zephyr Device Tree HOWTOs
   <https://docs.zephyrproject.org/4.0.0/build/dts/howtos.html#get-your-devicetree-and-generated-header>`_

Clock control
=============

STM32 domain clocks
-------------------

However, for peripherals that support domain clocks, clock source macros
``STM32_SRC_*`` and clock selection macros ``*_SEL(X)`` are used to determine
the clock source for the domain.

For example, to configure FDCAN1 for STM32G474RE to use PLLQ as the clock
source, the following code snippet is used:

.. code-block:: dts

   &fdcan1 {
       clocks = <&rcc STM32_CLOCK_BUS_APB1 0x02000000>,
                <&rcc STM32_SRC_PLL_Q FDCAN_SEL(1)>;
   };

``STM32_SRC_*`` is easy to determine, but ``*_SEL(X)`` is not. To determine it,
you have to refer to the clock configuration register (CCIPR) of the reset and
clock control (RCC) section in the reference manual, where the value of ``X`` is
listed in the table.

.. note::

   The default configurations in ``stm32*.dtsi`` may only define bus clock
   source, but you still have to copy it to your own device tree and add the
   domain clock of your choice.

Direct Memory Access (DMA)
==========================

Direct memory access (DMA) sees very limited support in Zephyr, especially in
documentation and samples. Currently only UART and SPI drivers has wide support
for DMA, throuth `UART async API
<https://docs.zephyrproject.org/3.6.0/reference/peripherals/uart.html#uart-async-api>`_
and SPI vendor specific Kconfig options `CONFIG_SPI_.*_DMA
<https://docs.zephyrproject.org/3.6.0/kconfig.html#!CONFIG_SPI_.*DMA>`_ ("wider"
support of eight vendors), and limited two vendors support for I\ :sup:`2`\ C
through `CONFIG_I2C_.*_DMA
<https://docs.zephyrproject.org/3.6.0/kconfig.html#!CONFIG_I2C_.*DMA>`_.

.. note::

   Support for UART async API can be checked by Kconfig option
   `CONFIG_SERIAL_SUPPORT_ASYNC
   <https://docs.zephyrproject.org/3.7.0/kconfig.html#CONFIG_SERIAL_SUPPORT_ASYNC>`_.

Since unlike UART has native API support for DMA, SPI and I\ :sup:`2`\ C drivers
may have some creative ways to utilize DMA. For example, for STM32 SPI, DMA is
used in sychronous API :c:func:`spi_transceive` (but not in async ones) to
context switch out current thread and let DMA handle the data transfer [#]_.

DMA in STM32
------------

Since STM32 series come with 4 iterations of DMA controllers, their
configurations will vary quite a lot. To use DMA, first we have to figure out
which DMA channel is connected to the peripheral that we want to enable DAM in
the DMA sections in the reference manual. Or for newer STM32 series that has DMA
multiplexers, please refer to the DMAMUX section such as table 91 for STM32G4
series.

.. note::
  
   The device tree configurations for DMA can be referenced from
   `zephyr/tests/drivers/uart/uart_async_api/boards
   <https://github.com/zephyrproject-rtos/zephyr/tree/v4.0.0/tests/drivers/uart/uart_async_api/boards>`_.

DAM with data cache
-------------------

High performance microcontrollers that have cache such as Cortex-M7 may cause
data inconsistency when using DMA since the CPU may read the data from the cache
whereas the DMA writes to the memory directly. This is typically resolved by
setting the memory region used for DMA buffer to non-cacheable in MPU or simply
turning the data cache off (setting `CONFIG_DCACHE
<https://docs.zephyrproject.org/4.0.0/kconfig.html#!CONFIG_DCACHE>`_\=n).

In Zephyr, you can get global non-cacheable memory using ``__nocache`` macro
defined in ``zephyr/include/zephyr/linker/section_tags.h`` after enabling the
`CONFIG_NOCACHE_MEMORY
<https://docs.zephyrproject.org/4.0.0/kconfig.html#!CONFIG_NOCACHE_MEMORY>`_
Kconfig option. Or allocate non-cacheable memory using
:c:func:`mem_attr_heap_alloc` as described in `Zephyr Memory Attributes
<https://docs.zephyrproject.org/4.0.0/services/mem_mgmt/index.html>`_

In STM32 driver implementation, memory buffer used to store data to
transmit/receive is directly used as DMA buffer [#]_, it requires changing the
application to ensure the memory buffer is non-cacheable. If the performance is
not a concern, turning the data cache off is a simpler solution.

The use of DMA is advised, but care must be taken to ensure the espected
behaviors.

Reference
---------

.. [#] `Zephyr STM32 SPI driver source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v3.6.0/drivers/spi/spi_ll_stm32.c#L1080>`_
   that uses DMA in synchronous API
.. [#] `Zephyr SMT32 UART driver source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/drivers/serial/uart_stm32.c#L1580>`_
   that set the DMA source address in async mode to the buffer

General Purpose Input/Output (GPIO)
===================================

Zephyr provides basic GPIO driver using the `GPIO API
<https://docs.zephyrproject.org/3.6.0/hardware/peripherals/gpio.html>`_ that can
perform basic operations such as digital read, write, and interrupt trigger.
However, for more advanced features such as LED effects and button debouncing,
you have to rely on higher-level drivers and subsystems. Below are two drivers
and subsystems that just do that:

Light Emitting Diode (LED)
--------------------------

Zephyr provides special `LED API
<https://docs.zephyrproject.org/3.6.0/hardware/peripherals/led.html>`_ that
controls various kinds of LEDs such as RGB LEDs and LED strips. Through
``gpio-leds`` device binding, you can control LEDs connected to GPIOs using the
LED API.

.. note::

   Since there may be multiple LEDs defined under the same ``gpio-leds`` device,
   the LED API requires ``LED number`` to specify which LED to control. And the
   ``LED number`` of a specific LED is the order it is defined in the
   ``gpio-leds`` device, **no matter if the LED is disabled or not** [#]_.

Input
-----

Zephyr provides special input subsystem designed for various kinds of inputs
such as key triggers, movement, etc through `Input API
<https://docs.zephyrproject.org/3.6.0/services/input/index.html>`_. It can also
be used for debouncing buttons through ``gpio-keys`` device binding. However,
currently it only supports callbacks APIs with no polling support.

.. note::

   Every children of ``gpio-keys`` devices must have a unique ``zephyr,code``
   property to identify the key. Available options start from `INPUT_KEY_RESERVED
   <https://docs.zephyrproject.org/3.6.0/services/input/index.html#c.INPUT_KEY_RESERVED>`_.

EXIT in STM32
-------------

The extended interrupt and event controller (EXIT) in STM32 is used for handling
interrupt events from GPIOs. Since every pin number is connected to a specific
EXIT line, only one GPIO with the same pin numbers can be used for external
interrupt triggers at a time [#]_. For example, since PA0 and PB0 share the same
pin number, only one of them can be used for external interrupt triggers.

.. note::

   Though EXIT input 0~15 for GPIOs does not map to NVIC IRQ numbers one-to-one
   (whcih means that they may share the same ISR), when the driver handlers the
   interrupt, it will check registers of EXIT to determine which pin triggered
   the interrupt and handle them accordingly [#]_.

Reference
---------

.. [#] `Zephyr GPIO LED driver source code that enumerates LEDs
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/drivers/led/led_gpio.c#L88>`_
.. [#] `Zephyr EXIT driver source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v3.7.0/drivers/interrupt_controller/intc_exti_stm32.c#L245>`_
.. [#] `Zephyr EXIT driver ISR source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v3.7.0/drivers/interrupt_controller/intc_exti_stm32.c#L170>`_

Pulse Width Modulation (PWM)
============================

For STM32 timers that support PWM generation using complementary output pins
(CHxN), STM32_PWM_COMPLEMENTARY flag must be set for that PWM channel in the
device tree. For example, to enable complementary output for TIM1 channel 1 in
STM32G4 series, the following code snippet is used:

.. code-block:: dts

   #include <dt-bindings/pwm/stm32_pwm.h>

   &tim1 {
       ...

       pwm1: pwm1 {
           pinctrl-0 = <&tim1_ch1n_pa7>;
           pinctrl-names = "default";
           status = "okay";
       };
   };
   ...

   &pwmleds {
       compatible = "pwm-leds";
       ...

       pwmled {
           pwms = <&pwm1 1 PWM_MSEC(1) STM32_PWM_COMPLEMENTARY>;
       };
   };

Universal Asynchronous Receiver/Transmitter (UART)
==================================================

STM32 UART provides hardware flow control for both RS232 and RS485 transceivers
(using ``CTS``, ``RTS`` pins for RS232 and ``DE`` pins for RS485). Since the
activation / deactivation time of the transceiver takes time, STM32 UART driver
provides a feature to delay the transmission of the first bit after toggling the
pins. For RS458 transceiver ``MAX487E`` that we used, it takes up to 3000ns to
finish the transaction [#]_. So for a baud rate of 115200, it will take 0.35 bit
time. With over sampling of 16 times per bit, it's 5.5 or minimum 6 sample time,
which cooresponds to ``de-assert-time`` and ``de-deassert-time`` in the device
tree.

Reference
---------

.. [#] MAX487E Datasheet, Switching Characteristics, Driver Disable Time from
   Low

CAN Bus
=======

The driver for controller area network (CAN) driver provides a nice feature of
figuring out the sync jump width and other parameters for the bus automatically,
you only need to provide the baud rate and the sampling point.

Error behaviors
---------------

In the overview section of the `Zephyr CAN controller documentation
<https://docs.zephyrproject.org/4.1.0/hardware/peripherals/can/controller.html>`_,
it mententioned the error behaviors of the CAN controller according to the ISO
11898-1. For CAN controllers that utilizes the `Bosch's M_CAN
<https://www.bosch-semiconductors.com/products/ip-modules/can-ip-modules/m-can/>`_
(incluing STM32's FDCAN controllers), when not in `CAN_MANUAL_RECOVERY
<https://docs.zephyrproject.org/4.1.0/doxygen/html/group__can__interface.html#ga3d8675253125b2af2bd22f0b2cc60cdd>`_
mode, the driver will automatically request the controller to recover from
bus-off state [#]_.

Since the receive error counter (``REC``) and the transmit error counter
(``TEC``) fields of the error counter register (``ECR``) are only up to 127 and
255 respectively [#]_, they should not be used to determine the error-passive
and bus-off states since those states are entered when the counter values exceed
127 and 255 respectively. Instead, the warning status (``EW``), error passive
(``EP``), and bus-off status (``BO``) bits of the protocol status register
(``PSR``) should be used to determine the error states. All of the above can be
accessed via :c:func:`can_get_state`.

Reference
---------

.. [#] `Zephyr M_CAN driver source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.1.0/drivers/can/can_mcan.c#L493>`_
   that automatically recovers from bus-off state
.. [#] `M_CAN user manual
   <https://www.bosch-semiconductors.com/media/ip_modules/pdf_2/m_can/mcan_users_manual_v331.pdf>`_
   where the error counter register (``ECR``) is described in section 2.3.13


Secure Digital Input Output (SDIO)
==================================

Typically, microcontrollers provide SDIO bus controllers to connect SD cards or 
MultiMedia cards such as `espressif,esp32-sdhc-slot
<https://docs.zephyrproject.org/4.0.0/build/dts/api/bindings/sdhc/espressif%2Cesp32-sdhc-slot.html#dtbinding-espressif-esp32-sdhc-slot>`_
native SDIO controller or SDIO in SPI mode `zephyr,sdhc-spi-slot
<https://docs.zephyrproject.org/4.0.0/build/dts/api/bindings/sdhc/zephyr%2Csdhc-spi-slot.html#dtbinding-zephyr-sdhc-spi-slot>`_
device bindings and they are marked as ``sd bus`` in Zephyr and implements the
`SDHC API
<https://docs.zephyrproject.org/4.0.0/hardware/peripherals/sdhc.html>`_. Such
API can then be used to connect to SD card using `zephyr,sdmmc-disk
<https://docs.zephyrproject.org/4.0.0/build/dts/api/bindings/sd/zephyr%2Csdmmc-disk.html#dtbinding-zephyr-sdmmc-disk>`_
or MMC using `zephyr,mmc-disk
<https://docs.zephyrproject.org/4.0.0/build/dts/api/bindings/sd/zephyr%2Cmmc-disk.html#dtbinding-zephyr-mmc-disk>`_
device bindings that implements `disk access API
<https://docs.zephyrproject.org/4.0.0/doxygen/html/group__disk__access__interface.html>`_
for file system.

However, currently STM32 drivers for SDIO does not expose the SDHC API, but
rather directly defines `st,stm32-sdmmc
<https://docs.zephyrproject.org/4.0.0/build/dts/api/bindings/mmc/st%2Cstm32-sdmmc.html#dtbinding-st-stm32-sdmmc>`_
device binging that directly implements the disk access API. This means that
STM32 microcontrollers are not able to connect other devices such as WiFi
modules that uses SDIO and cannot be tested by tests for SDHC controllers such
as ``tests/drivers/sdhc`` or ``tests/subsys/sd/sdmmc`` which requires generic
``zephyr,sdmmc-disk`` binding.

Battery Backed RAM (BBRAM)

==========================

Zephyr provides a battery backed RAM (BBRAM) driver that allows you to store
data across system resets through `BBRAM API
<https://docs.zephyrproject.org/3.6.0/hardware/peripherals/bbram.html>`_.
Depending on the hardware, the data may be persisted even if the main power is
lost, being kept by the dedicated battery, hence the name.

However, not all STM32 serise device tree include ``st,stm32-bbram`` device that
corrsepond to BBRAM. To use it, add it to ``st,stm32-rtc`` device in the device
tree overlay like so:

.. code-block:: dts

   &rtc {
       bbram: backup_regs {
           compatible = "st,stm32-bbram";
           st,backup-regs = <32>;
           status = "okay";
       };
   };

Where ``st,backup-regs`` is the number of backup register of the STM32 and
the exact values should refer to the reference manuals.

Real Time I/O (RTIO)
====================

`RTIO <https://docs.zephyrproject.org/4.0.0/services/rtio/index.html>`_ is a set
of interfaces inspired by Linux's ``io_uring`` that facilitates multiplexed
asynchronous I/O operations. After its adoption in 3.4.0, it has quickly become
the norm for defining new APIs for asynchronous I/O operations in Zephyr but
currently only includes I2C, SPI, and sensor drivers. Today still very few
drivers natively support RTIO (i.e. use DMA or other coprocessors for true
asynchronous transaction), threre are fallbacks that wraps the synchronous API
to RTIO API for the above three drivers [#]_ [#]_ [#]_.

The official documentation does not provide much information about the use of
RTIO, but you can refer to the code and comments in `I2C lookpack sample
<https://docs.zephyrproject.org/4.0.0/samples/drivers/i2c/rtio_loopback/README.html#i2c-rtio-loopback>`_
for a sample usage of RTIO and `RTIO reference
<https://docs.zephyrproject.org/4.0.0/doxygen/html/group__rtio.html>`_ for API
documentation.

Work Request
------------

Aside from relaying on interrupts to achieve non-blocking operations, RTIO also
provides work request API to dispatch work that requires blocking operations
such as the aforementioned fallbacks.

.. note::

   The work request API is neither documented in RTIO documentation nor in a
   doxygen group that can be referenced from the RTIO group. It's only available
   in `its file reference
   <https://docs.zephyrproject.org/4.0.0/doxygen/html/work_8h.html>`_.

References
----------

.. [#] `SPI driver source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/drivers/spi/spi_rtio.c>`_
   for RTIO fallback
.. [#] `I2C driver source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/drivers/i2c/i2c_rtio_default.c>`_
   for RTIO fallback
.. [#] `Sensor driver source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/drivers/sensor/default_rtio_sensor.c>`_
   for RTIO fallback

Sensors
=======

Asynchronous API
----------------

Sensor asynchronous driver API is built on top of RTIO, and its usage can be
referenced from the `sensor read and decode
<https://docs.zephyrproject.org/4.0.0/hardware/peripherals/sensor/read_and_decode.html>`_
documentation.

To create a new sensor driver that support asynchronous API, both the decoder
API and the async read initialization (:c:member:`sensor_driver_api.submit`) can
be referenced from `default_rtio_sensor.c
<https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/drivers/sensor/default_rtio_sensor.c>`_.
And since default implementation for ``submit`` does not support streaming [#]_,
the implementation of it can be referenced from existing drivers such as
`adxl345_stream.c
<https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/drivers/sensor/adi/adxl345/adxl345_stream.c>`_. 

References
----------

.. [#] `sensor_iodev_submit() source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/drivers/sensor/default_rtio_sensor.c#L25>`_
   that does not support streaming
