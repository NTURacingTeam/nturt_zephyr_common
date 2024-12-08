.. _notes_drivers:

==============
Zephyr Drivers
==============

Device Tree
===========

Using device tree to describe hardware configurations across different platforms
is a great design in my opinion, especially for microcontrollers where there are
lots of vendors with their unique features for similar peripherals. However,
just like Zephyr's disadvantages, the documentation and examples are few,
especially for vendor specific configurations. Here are some tips for using
device tree in Zephyr:

Device tree includes
--------------------

Decive tree supports including other device tree files just like how C does, but
without a language service provider (LSP) like IntelliSense for C/C++ or Pylance
for Python, it is hard to find where the included files are. The following are
some common places where Zephyr may put these files:

- Device tree header files (.dtsi) for a specific architecture in ``zephyr/dts``
- C header files (.h) for constants and macros in
  ``zephyr/include/zephyr/dt-bindings``
- Board support package (BSP) files in ``zephyr/boards``
- Vendor specific device tree files (especially for ping control) in
  ``modules/hal/<vendor>/dts``

Device tree macros
------------------

Since device tree in Zephyr is processed into C macros for the compiler to
further process, it supports including C hearder files as well as using macros
defined in it.

For example, you may see ``clock-frequency = <DT_FREQ_K(48)>;``, in clock
configs where ``DT_FREQ_K`` is a macro defined as ``DT_FREQ_M(x) ((x) * 1000)``
in ``zephyr/dts/common/freq.h``.

STM32 device tree
-----------------

For STM32, it is a good idea to use STM32CubeMX as a reference when configuring
the device tree as most of the time is just copying the configurations from
STM32CubeMX to the device tree.

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
  <https://github.com/zephyrproject-rtos/zephyr/tree/v4.0-branch/tests/drivers/uart/uart_async_api/boards>`_.

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
  <https://github.com/zephyrproject-rtos/zephyr/blob/v3.6-branch/drivers/spi/spi_ll_stm32.c#L1080>`_
  that uses DMA in synchronous API
.. [#] `Zephyr SMT32 UART driver source code
  <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0-branch/drivers/serial/uart_stm32.c#L1580>`_
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
  <https://github.com/zephyrproject-rtos/zephyr/blob/v4.0-branch/drivers/led/led_gpio.c#L88>`_
.. [#] `Zephyr EXIT driver source code
  <https://github.com/zephyrproject-rtos/zephyr/blob/v3.7-branch/drivers/interrupt_controller/intc_exti_stm32.c#L245>`_
.. [#] `Zephyr EXIT driver ISR source code
  <https://github.com/zephyrproject-rtos/zephyr/blob/v3.7-branch/drivers/interrupt_controller/intc_exti_stm32.c#L170>`_

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

CAN Bus
=======

The driver for controller area network (CAN) driver provides a nice feature of
figuring out the sync jump width and other parameters for the bus automatically,
you only need to provide the baud rate and the sampling point.

Weirdly, maximum baud rate for CAN bus is set to 800kbps in Zephyr [#]_.

Reference
---------

.. [#] `Zephyr CAN driver source code
  <https://github.com/zephyrproject-rtos/zephyr/blob/v3.6-branch/include/zephyr/drivers/can/can_mcan.h#L1318>`_
  that limits the maximum baud rate to 800kbps
